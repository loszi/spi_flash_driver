/**
 * @file ma_spi_flash_s25fl1xxk.h
 *
 * @brief FreeRTOS+FAT driver for Spansion S25FL1xxk SPI flash devices.
 * @author Lovas Szilárd <lovas.szilard@gmail.com>
 * @Version: 0.1
 * @note: initial version
 * @Date: 2017-01-03 13:00
 */

#ifndef FREERTOS_PLUS_FAT_PORTABLE_HERCULES_S25FL1XXK_MA_SPI_FLASH_S25FL1XXK_H_
#define FREERTOS_PLUS_FAT_PORTABLE_HERCULES_S25FL1XXK_MA_SPI_FLASH_S25FL1XXK_H_



#include "ma_spansion_s25fl1xxk_driver_config.h"
#include "ma_spansion_s25fl1xxk_command_set.h"


#define max(a,b) \
		({ __typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a > _b ? _a : _b; })

#define min(a,b) \
		({ __typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a < _b ? _a : _b; })



static int32_t prvSpansionSPI_FFRead( uint8_t *pucDestination,	/* Destination for data being read. */
							uint32_t ulSectorNumber,			/* Sector from which to start reading data. */
							uint32_t ulSectorCount,				/* Number of sectors to read. */
							FF_Disk_t *pxDisk );				/* Describes the disk being read from. */

static int32_t prvSpansionSPI_FFWrite( uint8_t *pucSource,		/* Source of data to be written. */
							uint32_t ulSectorNumber,			/* The first sector being written to. */
							uint32_t ulSectorCount,				/* The number of sectors to write. */
							FF_Disk_t *pxDisk );				/* Describes the disk being written to. */

/* Middle level functions. */
static uint8_t prvSpansionSPI_IsBusy(void);
static void prvSpansionSPI_WriteEnable(void);
static void prvSpansionSPI_WriteDisable(void);
static void prvSpansionSPI_WriteSector(uint8_t *pucSource, uint32_t xAddress, uint32_t xFatSectorsToWrite);
static void prvSpansionSPI_ChipErase(void);
static void prvSpansionSPI_SectorErase(uint32_t xAddress);
static uint8_t prvSpansionSPI_TestAndReadSector(uint8_t *pxDestination, uint32_t xAddress, uint8_t xMaxSectors);
static FF_Error_t prvPartitionAndFormatDisk( FF_Disk_t *pxDisk );

/* Low level functions. */
static uint8_t xSpiTransferByte(uint8_t xTX_Data);

#endif /* FREERTOS_PLUS_FAT_PORTABLE_HERCULES_S25FL1XXK_MA_SPI_FLASH_S25FL1XXK_H_ */
