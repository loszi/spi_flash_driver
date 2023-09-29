/**
 * @file ma_hercules_bit_banging.inc
 *
 * @brief FreeRTOS+FAT driver for Spansion S25FL1xxk SPI flash devices.
 * @author Lovas Szilárd <lovas.szilard@gmail.com>
 * @version: 0.1
 * @note: initial version
 * @date: 2017-03-28 10:00
 */


/* HALCoGen generated lowlevel API. */
#include "HL_spi.h"

#define spansionSPI_INTERFACE		spiREG5					/* SPI physical interface (SPI mode). */
#define spansionSPI_CS_SET()		spansionSPI_INTERFACE->PC5 = (1 << SPI_PIN_CS1)
#define spansionSPI_CS_CLEAR()		spansionSPI_INTERFACE->PC4 = (1 << SPI_PIN_CS1)
#define spansionSPI_CLK_SET()		spansionSPI_INTERFACE->PC4 = (1 << SPI_PIN_CLK)
#define spansionSPI_ENA_CLEAR()		spansionSPI_INTERFACE->PC5 = (1 << SPI_PIN_ENA)
#define spansionSPI_ENA_SET()		spansionSPI_INTERFACE->PC4 = (1 << SPI_PIN_ENA)
#define spansionSPI_CLK_CLEAR()		spansionSPI_INTERFACE->PC5 = (1 << SPI_PIN_CLK)
#define spansionSPI_SIMO_SET()		spansionSPI_INTERFACE->PC4 = (1 << SPI_PIN_SIMO)
#define spansionSPI_SIMO_CLEAR()	spansionSPI_INTERFACE->PC5 = (1 << SPI_PIN_SIMO)
#define spansionSPI_SOMI_GET()		(spansionSPI_INTERFACE->PC2 & ((uint32_t)(1 << SPI_PIN_SOMI_3)))
/* 4 wire mode */
#define spansionSPI_SOMI_0_GET()	(spansionSPI_INTERFACE->PC2 & ((uint32_t)(1 << SPI_PIN_SOMI)))
#define spansionSPI_SOMI_1_GET()	(spansionSPI_INTERFACE->PC2 & ((uint32_t)(1 << SPI_PIN_SOMI_1)))
#define spansionSPI_SOMI_2_GET()	(spansionSPI_INTERFACE->PC2 & ((uint32_t)(1 << SPI_PIN_SOMI_2)))
#define spansionSPI_SOMI_3_GET()	(spansionSPI_INTERFACE->PC2 & ((uint32_t)(1 << SPI_PIN_SOMI_3)))
#define spansionSPI_SOMI_0_CHECK(REG)	(REG & ((uint32_t)(1 << SPI_PIN_SOMI)))
#define spansionSPI_SOMI_1_CHECK(REG)	(REG & ((uint32_t)(1 << SPI_PIN_SOMI_1)))
#define spansionSPI_SOMI_2_CHECK(REG)	(REG & ((uint32_t)(1 << SPI_PIN_SOMI_2)))
#define spansionSPI_SOMI_3_CHECK(REG)	(REG & ((uint32_t)(1 << SPI_PIN_SOMI_3)))

#define spansionSPI_ENA_4_WIRE_MODE	1

/* Status register bits */
#define spansionSPI_SR1_BUSY_BIT	0x01
#define spansionSPI_SR2_QE_BIT		0x02

static uint8_t ucSpiTransferByte(uint8_t ucTX_Data);
static uint8_t ucSpiQuadReadByte(void);
static void prvSPI_CsDelay(void);

/**
 * @fn static uint8_t xSpiTransferByte(uint8_t xTX_Data)
 * @brief Low level function for send and receive one byte over GPIO with bit-banging.
 * @param [in] uint8_t byte to transfer
 * @return uint8_t received byte
 */
static uint8_t ucSpiTransferByte(uint8_t ucTX_Data)
{
	uint8_t i;
	uint8_t xRX_Data = 0;

	for(i=0; i<8; i++)
		{
		if(((ucTX_Data << i) & 0x80) == 0x80)
			{
			/* Set SIMO 1 */
			spansionSPI_SIMO_SET();
			}
		else
			{
			/* Set SIMO 0 */
			spansionSPI_SIMO_CLEAR();
			}
		/* CLK HIGH */
		spansionSPI_CLK_SET();
		/* READ SOMI */
		if(spansionSPI_SOMI_GET())
			{
			xRX_Data |= 0x80 >> i;
			}
		/* CLK LOW */
		spansionSPI_CLK_CLEAR();
		}

    return(xRX_Data);
}

/**
 * @fn static uint8_t xSpiQuadTransferByte(uvoid)
 * @brief Low level function for send and receive one byte over 4-wire GPIO with bit-banging.
 * @return uint8_t received byte
 */
static uint8_t ucSpiQuadReadByte(void)
{
	uint8_t i;
	uint8_t xRX_Data = 0;
	uint32_t uxPC2;

	/* SPI_ENA must be cleared before get SOMIx bits. */
	for(i=0; i<2; i++)
		{
		/* CLK HIGH */
		spansionSPI_CLK_SET();
		/* READ 4 SOMI bits */
		uxPC2 = spansionSPI_INTERFACE->PC2;
		/* CLK LOW */
		spansionSPI_CLK_CLEAR();
		if(spansionSPI_SOMI_0_CHECK(uxPC2))	/* bit 4, 0 */
			{
			xRX_Data |= 0x10 >> 4*i;
			}
		if(spansionSPI_SOMI_3_CHECK(uxPC2))	/* bit 5, 1 */
			{
			xRX_Data |= 0x20 >> 4*i;
			}
		if(spansionSPI_SOMI_1_CHECK(uxPC2))	/* bit 6, 2 */
			{
			xRX_Data |= 0x40 >> 4*i;
			}
		if(spansionSPI_SOMI_2_CHECK(uxPC2))	/* bit 7, 3 */
			{
			xRX_Data |= 0x80 >> 4*i;
			}
		}
    return(xRX_Data);
}

/**
 * @fn inline static void prvSPI_CsDelay(void))
 * @brief Place holder for CS delay.
 */
inline static void prvSPI_CsDelay(void)
{
/* Put some delay here, if its needed. */
}
