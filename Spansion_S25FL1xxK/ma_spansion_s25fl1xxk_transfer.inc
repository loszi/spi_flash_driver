/**
 * @file ma_spansion_s25fl1xxk_transfer.inc
 *
 * @brief FreeRTOS+FAT driver for Spansion S25FL1xxk SPI flash devices.
 * @author Lovas Szilárd <lovas.szilard@gmail.com>
 * @version: 0.1
 * @date: 2017-05-02 13:00
 * - initial version
 */

#define spansionSPI_ENA_4_WIRE_MODE	1

/* Status register bits */
#define spansionSPI_SR1_BUSY_BIT	0x01
#define spansionSPI_SR2_QE_BIT		0x02

static void prvSpansionSPI_SectorRead(uint8_t * pucDestination, uint32_t xSpiAddress);
static void prvSpansionSPI_SectorWrite(uint8_t *pucSource, uint32_t xSpiAddress);
static void prvSpansionSPI_SectorErase(uint32_t xSpiAddress);
static void prvSpansionSPI_WriteEnable(void);
static void prvSpansionSPI_QuadEnable(void);
static void prvSpansionSPI_ChipErase(void);
static BaseType_t prvSpansionSPI_IsChipEraseInProgress(void);
static uint8_t prvSpansionSPI_IsBusy(void);
static uint8_t prvSpansionSPI_ReadStatusRegister(uint8_t ucRegister);

static BaseType_t xChipEraseInProgress = pdFALSE;

/**
 * @fn static void prvSpansionSPI_SectorRead(uint8_t * pucDestination, uint32_t xSpiAddress)
 * @brief Read an SPI sector (4096 byte).
 * @param pucDestination Destination.
 * @param xSpiAddress SPI address.
 */
static void prvSpansionSPI_SectorRead(uint8_t * pucDestination, uint32_t xSpiAddress)
{
	uint16_t i;

	/* Trace macro. */
	traceSPI_FLASH_READ_SECTOR_START(xSpiAddress, 8, xGetHighResolutionTime());

	while(prvSpansionSPI_IsBusy());

	/* Activate _CS. */
	spansionSPI_CS_SET();

#if(spansionSPI_ENA_4_WIRE_MODE)
	/* Sends read data command. */
	ucSpiTransferByte(spansionFastReadQuadOutput);

	/* Sends address. */
	ucSpiTransferByte(xSpiAddress >> 16);
	ucSpiTransferByte(xSpiAddress >> 8);
	ucSpiTransferByte(xSpiAddress);
	/* dummy bits */
	ucSpiTransferByte(0);

	spansionSPI_ENA_CLEAR();
	/* Read data bytes. */
	for(i = 0; i < spansionSPI_SECTOR_SIZE; i++)
	{
		*(pucDestination++) = ucSpiQuadReadByte();
	}
	spansionSPI_ENA_SET();
#else
	/* Sends read data command. */
	ucSpiTransferByte(spansionReadData);

	/* Sends address. */
	ucSpiTransferByte(xSpiAddress >> 16);
	ucSpiTransferByte(xSpiAddress >> 8);
	ucSpiTransferByte(xSpiAddress);

	/* Read data bytes. */
	for(i = 0; i < spansionSPI_SECTOR_SIZE; i++)
	{
		*(pucDestination++) = ucSpiTransferByte(0x0000);
	}
#endif

	/* Deactivate _CS. */
	spansionSPI_CS_CLEAR();

	/* Trace macro. */
	traceSPI_FLASH_READ_SECTOR_END(xSpiAddress, 8, xGetHighResolutionTime());

}


/**
 * @fn static void prvSpansionSPI_SectorRead(uint8_t * pucDestination, uint32_t xSpiAddress)
 * @brief Write an SPI sector (4096 byte).
 * @param pucSource Source.
 * @param xSpiAddress SPI address.
 */
static void prvSpansionSPI_SectorWrite(uint8_t *pucSource, uint32_t xSpiAddress)
{

	uint32_t xAddress = xSpiAddress, xPage, i;
	uint8_t *pucIndex = pucSource;

	while(prvSpansionSPI_IsBusy())vTaskDelay(1);

	/* Trace macro. */
	traceSPI_FLASH_WRITE_SECTOR_START(xSpiAddress, 8, xGetHighResolutionTime());

	for(xPage = 0; xPage < spansionSPI_SECTOR_SIZE / spansionSPI_PAGE_SIZE; xPage++)
	{
		prvSpansionSPI_WriteEnable();

		/* Activate _CS. */
		spansionSPI_CS_SET();

		/* Sends page program command. */
		ucSpiTransferByte(spansionPageProgram);

		/* Sends address. */
		ucSpiTransferByte(xAddress >> 16);
		ucSpiTransferByte(xAddress >> 8);
		ucSpiTransferByte(xAddress);

		/* Send data bytes. */
		for(i = 0; i < spansionSPI_PAGE_SIZE; i++)
		{
			ucSpiTransferByte(*(pucIndex));
			pucIndex++;
		}

		/* Deactivate _CS. */
		spansionSPI_CS_CLEAR();

		/* Calculates page starting address */
		xAddress += spansionSPI_PAGE_SIZE;

		//TODO: Using FF_ERR_DRIVER_BUSY mechanism.
		//while(prvSpansionSPI_IsBusy())vTaskDelay(1);
		while(prvSpansionSPI_IsBusy());
	}

	/* Trace macro. */
	traceSPI_FLASH_WRITE_SECTOR_END(xSpiAddress, 8, xGetHighResolutionTime());
}

/**
 * @fn static void prvSpansionSPI_SectorErase(uint32_t xAddress)
 * @brief Erases that full Sector (4K) which contains the given address
 * @param [in] xAddress
 */
static void prvSpansionSPI_SectorErase(uint32_t xSpiAddress)
{
	xSpiAddress &= 0x00fff000;
	/* Trace macro. */
	traceSPI_FLASH_ERASE_SECTOR_START(xSpiAddress, 8, xGetHighResolutionTime());

	while(prvSpansionSPI_IsBusy())vTaskDelay(1);

	/* Sends write enable command before erasing chip. */
	prvSpansionSPI_WriteEnable();

	/* Activate _CS. */
	spansionSPI_CS_SET();

	/* Sends chip erase command. */
	ucSpiTransferByte(spansionSectorErase);

	/* Sends address. */
	ucSpiTransferByte(xSpiAddress >> 16);
	ucSpiTransferByte(xSpiAddress >> 8);
	ucSpiTransferByte(xSpiAddress);

	/* Deactivate _CS. */
	spansionSPI_CS_CLEAR();

	while(prvSpansionSPI_IsBusy())vTaskDelay(1);
	//while(prvSpansionSPI_IsBusy());

	/* Trace macro. */
	traceSPI_FLASH_ERASE_SECTOR_END(xSpiAddress, 8, xGetHighResolutionTime());
}

/**
 * @fn static void prvSpansionSPI_WriteEnable(void)
 * @brief Enable write access to the chip.
 */
static void prvSpansionSPI_WriteEnable(void)
{
	uint16_t xTransmitData;

	/* Activate _CS. */
	spansionSPI_CS_SET();

	/* Sends write enable command. */
	xTransmitData = spansionWriteEnable;
	ucSpiTransferByte(xTransmitData);

	/* Deactivate _CS. */
	spansionSPI_CS_CLEAR();
}

/**
 * @fn static void prvSpansionSPI_QuadEnable(void)
 * @brief Sets Quad Enable (QE) bit of Status Register2.
 */
static void prvSpansionSPI_QuadEnable(void)
{
	uint16_t xTransmitData;
	uint8_t ucStatusRegister1, ucStatusRegister2, ucStatusRegister3;

	ucStatusRegister1 =  prvSpansionSPI_ReadStatusRegister(spansionReadStatusRegister1);
	ucStatusRegister2 =  prvSpansionSPI_ReadStatusRegister(spansionReadStatusRegister2);
	ucStatusRegister3 =  prvSpansionSPI_ReadStatusRegister(spansionReadStatusRegister3);

	/* Sends "Write Enable for volatile Status Register (0x50)" command. */
	spansionSPI_CS_SET();
	xTransmitData = spansionWriteEnableForVolatileStatusRegister;
	ucSpiTransferByte(xTransmitData);
	spansionSPI_CS_CLEAR();

	/* Sends "Write Status Registers (0x01)" command. */
	prvSPI_CsDelay();
	spansionSPI_CS_SET();
	xTransmitData = spansionWriteStatusRegisters;
	ucSpiTransferByte(xTransmitData);

	ucSpiTransferByte(ucStatusRegister1);
	ucSpiTransferByte(ucStatusRegister2 | spansionSPI_SR2_QE_BIT);
	ucSpiTransferByte(ucStatusRegister3);

	spansionSPI_CS_CLEAR();
}

#if(0)
/**
 * @fn static void prvSpansionSPI_WriteDisable(void)
 * @brief Disable write access to the chip.
 */
static void prvSpansionSPI_WriteDisable(void)
{
	/* Activate _CS. */
	spansionSPI_CS_SET();

	/* Sends write disable command. */
	ucSpiTransferByte(spansionWriteDisable);

	/* Deactivate _CS. */
	spansionSPI_CS_CLEAR();
}
#endif

/**
 * @fn static void prvSpansionSPI_ChipErase(void)
 * @brief Erase the full chip in one step.
 */
static void prvSpansionSPI_ChipErase(void)
{

	while(prvSpansionSPI_IsBusy());
	xChipEraseInProgress = pdTRUE;

	/* Sends write enable command before erasing chip. */
	prvSpansionSPI_WriteEnable();

	/* Activate _CS. */
	spansionSPI_CS_SET();

	/* Sends chip erase command. */
	ucSpiTransferByte(spansionChipErase);

	/* Deactivate _CS. */
	spansionSPI_CS_CLEAR();
}

static BaseType_t prvSpansionSPI_IsChipEraseInProgress(void)
{
	if((pdTRUE == xChipEraseInProgress))
	{
		xChipEraseInProgress = prvSpansionSPI_IsBusy();
	}
	return(xChipEraseInProgress);
}

/**
 * @fn static uint8_t prvSpansionSPI_IsBusy(void)
 * @brief Check if device is executing Erase, Block Erase, Chip Erase,
 * Write Status Registers or Erase / Program Security Register command.
 * @return pdTRUE in case of device is busy
 */
static uint8_t prvSpansionSPI_IsBusy(void)
{
	/* BIT0: BUSY, Embedded Operation Status
	 * 0 = Not Busy, no embedded operation in progress,
	 * 1 = Busy, embedded operation in progress.
	 */
	return(prvSpansionSPI_ReadStatusRegister(spansionReadStatusRegister1) & spansionSPI_SR1_BUSY_BIT);
}

/**
 * @fn static uint8_t prvSpansionSPI_ReadStatusRegister1(uint8_t ucRegister)
 * @return SPI flash Status Register (1-3)
 * @param [in] ucRegister
 */
static uint8_t prvSpansionSPI_ReadStatusRegister(uint8_t ucRegister)
{
	uint16_t xReceiveData;

	spansionSPI_CS_SET();

	prvSPI_CsDelay();

	/* Send read status register command. */
	ucSpiTransferByte(ucRegister);

	/* Read status register value. */
	xReceiveData = ucSpiTransferByte(0x0000);

	prvSPI_CsDelay();
	spansionSPI_CS_CLEAR();
	prvSPI_CsDelay();
	return(xReceiveData);
}

