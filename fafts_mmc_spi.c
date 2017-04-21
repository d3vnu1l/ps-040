/******************************************************************************
 *
 *              SECURE DIGITAL MEDIA INTERFACE
 *
 ******************************************************************************
 * FileName:        fatfs_mmc_spi.c
 * Dependencies:    See include below
 * Processor:       PIC32
 * Compiler:        C32 V1.03
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 * It is 
 * your responsibility to ensure that your application meets with your 
 * specifications.Further, the implementation and use of the FAT file system 
 * accompanying this code, SD card specifications, MMC card specifications 
 * and other third party tools may require a license from various entities, 
 * including, but not limited to Microsoft® Corporation, SD Card Association 
 * and MMCA.  It is your responsibility to obtain more information regarding 
 * any applicable licensing obligations.
 * 
 * CODE AND INFORMATION ACCOMPANYING THIS MESSAGE IS PROVIDED “AS IS.”  
 * MICROCHIP AND ITS LICENSORS MAKES NO REPRESENTATION OR WARRANTIES OF ANY 
 * KIND WHETHER EXPRESS OR IMPLIED, WRITTEN OR ORAL, STATUTORY OR OTHERWISE, 
 * RELATED TO THE INFORMATION PROVIDED TO YOU, INCLUDING BUT NOT LIMITED TO 
 * ITS CONDITION, QUALITY, PERFORMANCE, MERCHANTABILITY, NON-INFRINGEMENT, 
 * OR FITNESS FOR PURPOSE.  MICROCHIP AND ITS LICENSORS ARE NOT LIABLE, UNDER 
 * ANY CIRCUMSTANCES, FOR SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES, 
 * FOR ANY REASON WHATSOEVER.  	 
 *
 *****************************************************************************
 *
 * 11/17/08	V1.00 D.Wenn		First version of fatfs sd card support
 *								Implements abstration layer
 *
 * 
 * 11/21/12     Jonas Meyer quitte@gmail.com added DMA support and other performance improvements
 * 
 * 
 *****************************************************************************/
#include <xc.h>
//#include "../../HardwareSpecific/HardwareProfile.h"
#include <p33EP512GM310.h>
#include "fatfs/src/diskio.h"

#include <GenericTypeDefs.h>
#include <plib.h>

/* Definitions for MMC/SDC command */
#define SD_CMD_IDLE					(0x40+0)	/* GO_IDLE_STATE */
#define SD_CMD_OP_COND				(0x40+1)	/* SEND_OP_COND (MMC) */
#define SD_CMD_IF_COND				(0x40+8)	/* SEND_IF_COND */
#define SD_CMD_CSD					(0x40+9)	/* SEND_CSD */
#define SD_CMD_CID					(0x40+10)	/* SEND_CID */
#define SD_CMD_STOP_TRANSMISSION	(0x40+12)	/* STOP_TRANSMISSION */
#define SD_CMD_SET_BLOCKLEN			(0x40+16)	/* SET_BLOCKLEN */
#define SD_CMD_READ_SINGLE_BLOCK	(0x40+17)	/* READ_SINGLE_BLOCK */
#define SD_CMD_READ_MULTIPLE_BLOCK	(0x40+18)	/* READ_MULTIPLE_BLOCK */
#define SD_CMD_SET_BLOCK_COUNT		(0x40+23)	/* SET_BLOCK_COUNT (MMC) */
#define SD_CMD_WRITE_BLOCK			(0x40+24)	/* WRITE_BLOCK */
#define SD_CMD_WRITE_MULTIPLE_BLOCK	(0x40+25)	/* WRITE_MULTIPLE_BLOCK */
#define SD_CMD_APP_CMD				(0x40+55)	/* APP_CMD */
#define SD_CMD_READ_OCR				(0x40+58)	/* READ_OCR */
#define SD_ACMD_OP_COND						(0xC0+41)	/* SEND_OP_COND (SDC) */
#define SD_ACMD_STATUS						(0xC0+13)	/* SD_STATUS (SDC) */
#define SD_ACMD_SET_WR_BLOCK_ERASE_COUNT	(0xC0+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */

/****************************************************************************
  Section:  Local Function Protoypes
  ***************************************************************************/
void Delay( DWORD count );
static BYTE SDSendCmd(BYTE cmd, DWORD arg);

/****************************************************************************
  Section:  Local Variable Defines
  ***************************************************************************/
static volatile DSTATUS Stat = STA_NOINIT; // disk status
static UINT CardType; // b0:MMC, b1:SDv1, b2:SDv2, b3:Block Addressing

//until I figure out a proper way to get the i2s buffer start address in here:
static BYTE * i2s_buffer;
static UINT i2s_buffersize;
static UINT newfile;

/****************************************************************************
  Section:  Inline Fucntions used by this module only
  ***************************************************************************/
static inline __attribute__((always_inline)) void SleepMS(unsigned int ms)
{
        //4 Timer1 Ticks are 1ms with a prescaler of 8@32.768khz
        WritePeriod1(0xffff);//ensure the timer doesn't wrap around before the period is set
        WriteTimer1(0);
        WritePeriod1(156*ms);
        mT1ClearIntFlag();
        while(!mT1GetIntFlag());
}

static inline __attribute__((always_inline)) unsigned char SPICalutateBRG(unsigned int pb_clk, unsigned int spi_clk)
{
    unsigned int brg;

    brg = pb_clk / (2 * spi_clk);

    if(pb_clk % (2 * spi_clk))
        brg++;

    if(brg > 0x100)
        brg = 0x100;

    if(brg)
        brg--;

    return (unsigned char) brg;
}

static inline __attribute__((always_inline)) void OpenSPI(unsigned int config1, unsigned int config2)
{
#if defined (MEDIASD_IF_SPI1)
    OpenSPI1(config1, config2);
#else
    OpenSPI2(config1, config2);
    
    SPI2CONbits.ON=0;
    SPI2CONbits.ENHBUF=1;
    //SPIxTXIF is set when the buffer is not full (has one or more empty elements)
    SPI2CONbits.STXISEL=0b11;
    SPI2CONbits.SRXISEL=0b01;
    //ignore receive overflows, this should be done dynamically for dma transfers only.
    SPI2CON2bits.IGNROV=1;
    SPI2CONbits.ON=1;

    DCRCCON=0x00;
                //do an Endian byte swap on word boundaries (i.e., reverse source byte order)
                DCRCCONbits.BYTO=0b01;
                DCRCCONbits.WBO=1;
                CrcAttachChannel(2, 0);
                DCRCCONbits.CRCEN=1;
#endif
}

static inline  __attribute__((always_inline)) void CloseSPI(void)
{
#if defined (MEDIASD_IF_SPI1)
    CloseSPI1();
#else
    SPI2CON=0x00;
    //CloseSPI2();
#endif
}

static inline  __attribute__((always_inline)) void putcSPI(unsigned int data_out)
{
#if defined (MEDIASD_IF_SPI1)
    putcSPI1(data_out);
#else
    while(SPI2STATbits.SPITBF);
    SPI2BUF=data_out;
#endif
}

static inline  __attribute__((always_inline)) unsigned int getcSPI(void)
{
#if defined (MEDIASD_IF_SPI1)
    return getcSPI1();
#else
    while(SPI2STATbits.SPIRBE); //for some reason this actually needs to be done
    return SPI2BUF;
#endif
}

static inline  __attribute__((always_inline)) void ChangeSPIBRG(unsigned int brg)
{
#if defined (MEDIASD_IF_SPI1)
    SpiChnSetBrg(1, brg);
#else
    SpiChnSetBrg(2, brg);
#endif
}

///////////////////////////////////////////////////////////////////
// transmit a byte of data on the SPI bus and read the returned result
static inline  __attribute__((always_inline)) BYTE SDTransferByte(BYTE SendByte)
{
    WriteSPI2((unsigned int) SendByte);//assume there are free elements in the TX Buffer blindly.
    while(SPI2STATbits.SPIRBE); //wait for the RX BUf to contain data
    return((BYTE)SPI2BUF);
}

///////////////////////////////////////////////////////////////////
// wait until the device is ready or timeout
static inline __attribute__((always_inline)) BYTE SDWaitRdy(void)
{
	BYTE res=0;
	
        //188 Timer23 Ticks are 1ms with a 256 prescaler @ 48Mhz
        WritePeriod23(0xffffffff);//ensure the timer doesn't wrap around before the period is set
        WriteTimer23(0);
        WritePeriod23(188*500); // wait for a timeout of 500ms
        mT23ClearIntFlag();

        WriteSPI2(0xFF);//one more write than reads in the beginning to keep the rx buffer filled
	while((res != 0xFF) && !mT23GetIntFlag()){
            WriteSPI2(0xFF);
            res=getcSPI();
        }
        getcSPI(); //flush the extra rx buffer element
	
	return res;
}

///////////////////////////////////////////////////////////////////
// Transmit a block of data, assumes always 512 bytes
#if _READONLY == 0
inline __attribute__ ((always_inline)) BOOL SDBufferTransmit(const BYTE *buffer, BYTE token)
{
	BYTE resp;
	UINT blockCount = 512;

    
    if (SDWaitRdy() != 0xFF)
    	return FALSE;
    
    // send data token
    SDTransferByte(token);
    
    if (token != 0xFD) {

        //wait for the TXBUF to be not full
        while(SPI2STATbits.SPITBF);

	//Use DMA for a 512Byte transfer to the SD-Card
        DCH1CON=0x02; // channel off, priority 2
        DCH1ECON=(_SPI2_TX_IRQ <<8|0x10); // start irq is SPI2 TX, start cell transfer on interrupt
        DmaChnSetTxfer(1,buffer,(void*)&SPI2BUF,blockCount,1,1);
        DmaChnStartTxfer(1,DMA_WAIT_BLOCK,0);

        //Discard CRC and get the return code
        while(13<SPI2STATbits.TXBUFELM);//wait for the buffer to have 3 free slots
        WriteSPI2(0xFF);
        WriteSPI2(0xFF);
        WriteSPI2(0xFF);
        while(!SPI2STATbits.SPIRBE){
            resp=ReadSPI2();
        }

        if ((resp & 0x1F) != 0x05)
                return FALSE;
    }
    
    return TRUE;
}
#endif

///////////////////////////////////////////////////////////////////
// read a block of data from the card (must be blocks of 4 bytes)
inline __attribute__((always_inline)) BOOL SDBufferReceive(BYTE *buffer, UINT size)
{
	BYTE token;
	DWORD_VAL data;

	_SetChipSelect(TRUE);
	// read the data token

        //4 Timer1 Ticks are 1ms with a prescaler of 8@32.768khz
        WritePeriod1(0xffff);//ensure the timer doesn't wrap around before the period is set
        WriteTimer1(0);
        WritePeriod1(4*100);   //100ms timeout
        mT1ClearIntFlag();
	do {
		token = SDTransferByte(0xFF);
	} while ((token == 0xFF) && !mT1GetIntFlag());

	// if invalid token return error
	if (token != 0xFE)
		return FALSE;

	

    if (size) {
            // change to 32 bit mode for the SPI to increase speed
            SPI2CONINV = _SPI2CON_MODE32_MASK | _SPI2CON_MODE16_MASK;
            DCH2CON=0x02; // channel off, priority 2
            DCH2INTbits.CHBCIF=0;
            DCH2ECON=(_SPI2_RX_IRQ <<8|0x10); // start irq is SPI2 RX, start cell transfer on interrupt
            DmaChnSetTxfer(2,(void*)&SPI2BUF,buffer,4,size,4);
            DmaChnEnable(2);
            size >>= 2;

            for(;size;size--)putcSPI(0xFFFFFFFF);

            while(!DCH2INTbits.CHBCIF);

            // return the SPI channel to 8 bit mode
            SPI2CONINV = _SPI2CON_MODE32_MASK | _SPI2CON_MODE16_MASK;
    }

	// read and discard the CRC
        SPI2CONINV = _SPI2CON_MODE16_MASK;
        WriteSPI2(0xFFFF);
        getcSPI();
        SPI2CONINV = _SPI2CON_MODE16_MASK;        

    return TRUE;
}

/******************************************************************************
 * Function:        DSTATUS MMC_disk_initialize(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Status after init
 *
 * Side Effects:    None
 *
 * Overview:        Function to initialize the SD Media called during mount
 *
 * Note:            None
 *****************************************************************************/
DSTATUS MMC_disk_initialize(void)
{ 
    BYTE resp, n, cmd;
    BYTE bData[4];
	unsigned long fpb;
	
    // Setup the I/O
    SDSetCDDirection();     // CD as input
    SDSetWEDirection();     // WE as input

	if (Stat & STA_NODISK)
		return Stat;		// no card in the socket

	fpb = FOSC / (1 << mOSCGetPBDIV());
	
    SleepMS(5);	
	_SetChipSelect(FALSE); 
    SleepMS(5);	
	OpenSPI(FATFS_SPI_START_CFG_1, FATFS_SPI_START_CFG_2);
    SleepMS(5);	        
    // set the baud rate to 400kHz
    ChangeSPIBRG(SPICalutateBRG(fpb, 400000));	
    SleepMS(5);	        

    // send '1' 80 bits with CS de-asserted
    for (n = 10; n; n--)
    	SDTransferByte(0xFF);
    
    // try to make the card enter idle state
    CardType = 0;
    if (SDSendCmd(SD_CMD_IDLE, 0) == 1) {
        //188 Timer23 Ticks are 1ms with a 256 prescaler at 48Mhz
        WritePeriod23(0xffffffff);//ensure the timer doesn't wrap around before the period is set
        WriteTimer23(0);
        WritePeriod23(188*1000);    //1000ms timeout
        mT23ClearIntFlag();
    	    	
	    // check for SDHC card
	    if (SDSendCmd(SD_CMD_IF_COND, 0x1AA) == 1) {
		    // accepted, so get trailing data of R7 response
			for (n = 0; n < 4; n++) 
				bData[n] = SDTransferByte(0xFF);
					
		    if ((bData[2] == 0x01) && (bData[3] == 0xAA)) {
				// card is SDC V2 can work at 2.7 to 3.6Volts   
				// Wait for leaving idle state (ACMD41 with HCS bit)
				while (!mT23GetIntFlag() && SDSendCmd(SD_ACMD_OP_COND, 1UL << 30));
				
				// Check CCS bit in the OCR
				if (!mT23GetIntFlag() && (SDSendCmd(SD_CMD_READ_OCR, 0) == 0)) {
					for (n = 0; n < 4; n++) 
						bData[n] = SDTransferByte(0xFF);
						
					CardType = (bData[0] & 0x40) ? 12 : 4;
				} 		  
				ChangeSPIBRG(SPICalutateBRG(fpb, 25000000)); 
			} 		    
		} else {
			// card is SDSC or MMC			
			if (SDSendCmd(SD_ACMD_OP_COND, 0) <= 1) 	{
				// SDC
				CardType = 2; 
				cmd = SD_ACMD_OP_COND;			
				ChangeSPIBRG(SPICalutateBRG(fpb, 25000000)); 
			} else {
				// MMC
				CardType = 1; 
				cmd = SD_CMD_OP_COND;			
				ChangeSPIBRG(SPICalutateBRG(fpb, 20000000)); 
			}
			
			// wait for the card to leave idle stae
			while (!mT23GetIntFlag() && SDSendCmd(cmd, 0));
			// set block length to 512
			if (mT23GetIntFlag() || SDSendCmd(SD_CMD_SET_BLOCKLEN, 512) != 0)
				CardType = 0;
		}		
	}   
    _SetChipSelect(FALSE);
    SDTransferByte(0xFF);
    
    // success
 	if (CardType != 0) {
 		Stat &= ~STA_NOINIT;
 	} else {
 		CloseSPI();
 	}		   
    _SetChipSelect(FALSE); 
       
    return Stat;
}  

/******************************************************************************
 * Function:        DSTATUS MMC_disk_status(void)
 *
 * PreCondition:    None
 *
 * Input:           drv - Physical drive number
 *
 * Output:          Status of drive
 *
 * Side Effects:    None
 *
 * Overview:        Function to return status of drive
 *
 * Note:            None
 *****************************************************************************/
DSTATUS MMC_disk_status(void)
{
	return Stat;	
}

/******************************************************************************
 * Function:        DRESULT MMC_disk_read(BYTE *buff,
 *									  DWORD sector,
 *									  BYTE count)
 *
 * PreCondition:    None
 *
 * Input:           buff - pointer to the data buffer to store read data
 *					sector - start sector number (LBA)
 *					count - sector count (1..255)
 *
 * Output:          Status of read
 *
 * Side Effects:    None
 *
 * Overview:        Function to read a specific sector on the media.
 *
 * Note:            None
 *****************************************************************************/
DRESULT MMC_disk_read(BYTE *buff, DWORD sector, BYTE count)
{
    WORD temp, Cntr;
    volatile unsigned int __time;

	if (!count)
		return RES_PARERR;
		
	if (Stat & STA_NOINIT)
		return RES_NOTRDY;

    // convert to byte address if required
    if (!(CardType & 8))
    	sector *= 512;
    	
    if (count == 1) {
	    // single block read
	 	if ((SDSendCmd(SD_CMD_READ_SINGLE_BLOCK, sector) == 0)
	 		&& (SDBufferReceive(buff, 512)))
	 		count = 0;
	} else {
		// multiple block read
		if (SDSendCmd(SD_CMD_READ_MULTIPLE_BLOCK, sector) == 0) {
			do {	
				if (!SDBufferReceive(buff, 512))
					break;
				buff += 512;
			} while (--count);
			SDSendCmd(SD_CMD_STOP_TRANSMISSION, 0);
		}
	}
      
	_SetChipSelect(FALSE);
    SDTransferByte(0xFF);
	
    return count ? RES_ERROR : RES_OK;
}  

/******************************************************************************
 * Function:        DRESULT MMC_disk_write(const BYTE *buff,
 *									   DWORD sector,
 *									   BYTE count)
 *
 * PreCondition:    None
 *
 * Input:           buff - Pointer to the data to be written
 *					sector - Start sector number (LBA)
 *					count - Sector count (1..255)
 *
 * Output:          Status of write
 *
 * Side Effects:    None
 *
 * Overview:        Function to write a specific sector on the media.
 *
 * Note:            None
 *****************************************************************************/
#if _READONLY == 0
DRESULT MMC_disk_write(const BYTE *buff, DWORD sector, BYTE count)
{
    WORD temp;

	if (!count)
		return RES_PARERR;
		
	if (Stat & STA_NOINIT)
		return RES_NOTRDY;
		
	if (Stat & STA_PROTECT)
		return RES_WRPRT;
		
	if (!(CardType & 8))
		sector *= 512;

	if (count == 1) {
		// single block write
		if ((SDSendCmd(SD_CMD_WRITE_BLOCK, sector) == 0) 
			&& (SDBufferTransmit(buff, 0xFE)))
			count = 0;
	} else {
		// multiple block write
		if (CardType & 6) 
			SDSendCmd(SD_ACMD_SET_WR_BLOCK_ERASE_COUNT, count);
		if (SDSendCmd(SD_CMD_WRITE_MULTIPLE_BLOCK, sector) == 0) {
			do {
				if (!SDBufferTransmit(buff, 0xFC))
					break;	
				buff += 512;
			} while (--count);
			// Send stop trans token
			if (!SDBufferTransmit(0, 0xFD))
				count = 1;	
		}
	}
	_SetChipSelect(FALSE);
    SDTransferByte(0xFF);
	
	return count ? RES_ERROR : RES_OK;
}
#endif

/******************************************************************************
 * Function:        DRESULT MMC_disk_ioctl(BYTE ctrl, void* buff)
 *
 * PreCondition:    None
 *
 * Input:           ctrl - control code
 *					buff - buffer to send/receive data block
 *
 * Output:          Success/Failure
 *
 * Side Effects:    None
 *
 * Overview:        Perform miscellaneous control functions
 *
 * Note:            None
 *****************************************************************************/
DRESULT MMC_disk_ioctl(BYTE ctrl, void* buff)
{
	DRESULT res;
	BYTE n, csd[16], *ptr;
	DWORD csize;
	
	ptr = buff;	

//    if(ctrl == TIMER_UPDATE)
//        disk_timerproc();
	
	if (Stat & STA_NOINIT)
		return RES_NOTRDY;
		
	res = RES_ERROR;
	switch (ctrl) {
		case CTRL_SYNC:
			_SetChipSelect(TRUE);		// flush buffers
			if (SDWaitRdy() == 0xFF)
				res = RES_OK;
			break;
			
		case GET_SECTOR_COUNT:			// get number of sectors on disk
			if ((SDSendCmd(SD_CMD_CSD, 0) == 0) &&
				(SDBufferReceive(csd, 16))) {
				if ((csd[0] >> 6) == 1) {
					// SDC ver 2.0
					csize = csd[9] + ((WORD) csd[8]	 << 8) + 1;
					*(DWORD*)buff = (DWORD) csize << 10;
				} else {
					// MMC or SD Ver1
					n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
					csize = (csd[8] >> 6) + ((WORD) csd[7] << 2) + 
								((WORD) (csd[6] & 3) << 10) + 1;
					*(DWORD*)buff = (DWORD)csize << (n - 9);
				}
				res = RES_OK;		
			}
			break;
			
		case GET_SECTOR_SIZE : // get size of sectors on disk 
			*(WORD*) buff = 512;
			res = RES_OK;
			break;
			
		case GET_BLOCK_SIZE : 		// get erase block size in units of sectors
			if (CardType & 4) {
				// SDC V2
				if (SDSendCmd(SD_ACMD_STATUS, 0) == 0) {
					SDTransferByte(0xFF);
					if (SDBufferReceive(csd, 16)) { // read partial data
						for(n = 64 - 16; n; n--)
							SDTransferByte(0xFF); // purge trailing data
						*(DWORD*)buff = 16UL << (csd[10] >> 4);
						res = RES_OK;
					}
				}
			} else {
				// SD ver 1 or MMC
				if ((SDSendCmd(SD_CMD_CSD, 0) == 0) &&
					(SDBufferReceive(csd, 16))) {
					if (CardType & 2) {
						*(DWORD*)buff = (((csd[10] & 63) << 1) +
										((WORD)(csd[11] & 128) >> 7) + 1) <<
										((csd[13] >> 6) - 1);
					} else {
						*(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) *
										(((csd[11] & 3) << 3) +
										((csd[11] & 224) >> 5) + 1);
					}
					res = RES_OK;		
				}
			}
			break;
			
		case MMC_GET_TYPE:	// get card types
			*ptr = CardType;
			res = RES_OK;
			break;
						
		case MMC_GET_CSD:	// get CSD block
			if ((SDSendCmd(SD_CMD_CSD, 0) == 0) 
				&& (SDBufferReceive(buff, 16))) {
				res = RES_OK;
			}
			break;
			
		case MMC_GET_CID:	// get CIF block
			if ((SDSendCmd(SD_CMD_CID, 0) == 0)
				&& (SDBufferReceive(buff, 16))) {
				res = RES_OK;
			}
			break;
			
		case MMC_GET_OCR:	// receive OCR
			if (SDSendCmd(SD_CMD_READ_OCR, 0) == 0) {
				for(n = 0; n < 4; n++)
					*((BYTE*)buff + n) = SDTransferByte(0xFF);
				res = RES_OK;
			}
			break;
			
		case MMC_GET_SDSTAT: 	// receive SD status
			if (SDSendCmd(SD_ACMD_STATUS, 0) == 0) {
				SDTransferByte(0xFF);
				if (SDBufferReceive(buff, 64))
					res = RES_OK;
			}
			break;
		
        default:
			res = RES_PARERR;	
	}
	
	_SetChipSelect(FALSE);
    SDTransferByte(0xFF);	
	return res;
}

/*********************************************************************
 * Function:  		static BYTE SDSendCmd(BYTE cmd, DWORD arg)     
 *
 * PreCondition:    
 *
 * Input:           cmd - command to be send
 *					arg - argument
 *
 * Output:          R1 response
 *
 * Side Effects:    
 *
 * Overview:        Send a command to the SPI device
 *
 * Note:
 * This function breaks in weird ways when trying to keep the TX and RX buffers
 * filled. My guess is that what it does is timing critical and only
 * works by chance
 ********************************************************************/
static BYTE SDSendCmd(BYTE cmd, DWORD arg)
{
	DWORD_VAL dVal;
	BYTE n, res;
	
	// if we are sending an app command then we must prefix this with CMD55
	if (cmd & 0x80) {
		cmd &= 0x7F;
		res = SDSendCmd(SD_CMD_APP_CMD, 0);
		if (res > 1)
			return res;
	}
	
	_SetChipSelect(FALSE);
	_SetChipSelect(TRUE);
	if (SDWaitRdy() != 0xFF)
		return 0xFF;
	//assume an empty enough TX BUF
	dVal.Val = arg;
	WriteSPI2(cmd);		// command
	WriteSPI2(dVal.v[3]);		// argument
	WriteSPI2(dVal.v[2]);
	WriteSPI2(dVal.v[1]);
	WriteSPI2(dVal.v[0]);
	n = 0x01;			// dummy CRC + stop	
	if (cmd == SD_CMD_IDLE) // hardcoded CRC for specific commands
		n = 0x95;
	if (cmd == SD_CMD_IF_COND)
		n = 0x87;
	WriteSPI2(n);
        //now remove those 6 bytes from the RXBUF
        getcSPI();getcSPI();getcSPI();
        getcSPI();getcSPI();getcSPI();
		
	// skip a stuff byte when stopping reading
	if (cmd == SD_CMD_STOP_TRANSMISSION)
		SDTransferByte(0xFF);
	
	n = 10;

        do {
            WriteSPI2(0xFF);
            res = getcSPI();
        }while(res & 0x80);
//***	} while ((res & 0x80) && --n);
	
	return res;
} 

///******************************************************************************
// * Function:        void disk_timerproc(void)
// *
// * PreCondition:    None
// *
// * Input:           None
// *
// * Output:          None
// *
// * Side Effects:    None
// *
// * Overview:        Perform 1ms updates of the disk io system
// *
// * Note:            None
// *****************************************************************************/
//void disk_timerproc(void)
//{
//	static WORD pv;	// present state variable
//	WORD p;
//	BYTE s;
//	UINT n;
//
//	// decrement the local timer counters
//	n = _Timer1;
//	if (n)
//		_Timer1--;
//
//	n = _Timer2;
//	if (n)
//		_Timer2--;
//
//	p = pv;
//	pv = SD_CD | SD_WE;  // check the input pins
//
//	if (p == pv) {
//		s = Stat;
//
//		if (p & BIT_0)
//			s |= STA_PROTECT;
//		else
//			s &= ~STA_PROTECT;
//
//		if (p & BIT_1)
//			s |= (STA_NODISK | STA_NOINIT);
//		else
//			s &= ~ STA_NODISK;
//
//		Stat = s;
//	}
//}
//
//
//

/******************************************************************************
 * Function:        void disk_cardstate(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Check for Card insertion and Write lock
 *
 * Note:            None
 *****************************************************************************/
void disk_cardstate(void)
{
    //this function doesn't actually check card presence but is hardcoded to inserted and writable
	static WORD pv;	// present state variable
	WORD p;
	BYTE s;
	UINT n;

	p = pv;
        //move to WL instead of WE: Write Lock is free of inversions(check!) with the used hardware
	pv = SD_CD | SD_WE;  // check the input pins

	if (p == pv) {
		s = Stat;

		if (p & BIT_0)
			s |= STA_PROTECT;
		else
			s &= ~STA_PROTECT;

		if (p & BIT_1)
			s |= (STA_NODISK | STA_NOINIT);
		else
			s &= ~ STA_NODISK;

		Stat = s;
	}
}

#if _READONLY == 0
DRESULT MMC_disk_write_i2s(const BYTE *buff, DWORD sector, BYTE count)
{
    static INT BufFilled;
    //the buffer offset is local to this function
    static UINT offset;
    static UINT ChunkSize;

    if (!(CardType & 8))
            sector *= 512;

    //decrement the number of Clusters written ignoring Overflows
    if(newfile)
        newfile--;

    // multiple block write
    if (CardType & 6)
            SDSendCmd(SD_ACMD_SET_WR_BLOCK_ERASE_COUNT, count);
    if (SDSendCmd(SD_CMD_WRITE_MULTIPLE_BLOCK, sector) == 0) {
            do {
                do{//wait for buffer to contain at least 512 byte and update timeout
                    BufFilled=(((INT)DCH0DPTR-(INT)offset));
                    if(BufFilled<0)
                        BufFilled=BufFilled+i2s_buffersize;
                    ChunkSize=(BufFilled+BufFilled>>2)-10;//1.25 statt 1.23, dafür -10, was 0.02*Sector entspricht.
                    //                      für die folgenden Sektoren verlasse ich mich auf den SPI BUS Overhead
                }while(!(ChunkSize&0xfffffe00));
            

                    WriteTimer45(0);
                    WritePeriod45((83*(i2s_buffersize-BufFilled))+(ChunkSize<<4));//(ChunkSize<<4) bedeutet 16*Chunksize
                            //check the timeout
                    if(IFS0bits.T5IF&&(!newfile))
//                        return RES_OVERFLOW;
                    IFS0bits.T5IF=0;

               do{
                    if (!SDBufferTransmit(i2s_buffer+offset, 0xFC))
                            break;
                    offset += 512;
                    ChunkSize-=512;
                    if(offset==i2s_buffersize)
                        offset=0;
                    count--;
               }while((ChunkSize&0xfffffe00)&&count);

            } while (count);
            // Send stop trans token
            if (!SDBufferTransmit(0, 0xFD))
                    count = 1;
    }
	
	_SetChipSelect(FALSE);
    SDTransferByte(0xFF);


	return count ? RES_ERROR : RES_OK;
}
#endif

void MMC_init_i2s_buffer(BYTE * buffer,UINT buffsize){
    i2s_buffer = buffer;
    i2s_buffersize = buffsize;
    newfile=30; //this ensures multiblock transfers are written 30 times ignoring timeouts. so 30*32KB.
}
