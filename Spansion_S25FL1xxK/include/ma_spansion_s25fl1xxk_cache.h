/**
 * @file ma_spi_flash_cache.h
 *
 * @brief WriteBack cache API for SPI flash devices.
 * @author Lovas Szilárd <lovas.szilard@gmail.com>
 * @Version: 0.1
 * @Date: 2017-02-16 14:00
 * - 0.1 - initial version
 */

#ifndef MA_SPI_FLASH_CACHE_H_
#define MA_SPI_FLASH_CACHE_H_

#include <stdint.h>
#include "ma_spansion_s25fl1xxk_command_set.h"

typedef enum {INVALID, VALID, MODIFIED, INCOMPATIBLE} cache_state_t;
typedef uint64_t cache_stamp_t;
typedef int8_t	cache_index_t;
typedef uint8_t cache_size_t;

typedef struct {
	uint32_t		Tag;					/* Cache Tag (Address). */
	cache_state_t	State;					/* Cache State. */
	cache_stamp_t	Stamp;					/* Time stamp for Last usage for LRU replacement. */
	uint8_t			Line[cacheLINE_SIZE];	/* Cache data. */
} cache_entry_t;

typedef struct {
	uint32_t		WriteHits;
	uint32_t		WriteMisses;
	uint32_t		ReadHits;
	uint32_t		ReadMisses;
	uint32_t		Hits;					/* Write + Read hits.*/
	uint32_t		Misses;					/* Write + Read misses.*/

} cache_stats_t;


static int32_t prvSpansionSPI_FFRead( uint8_t *pucDestination,	/* Destination for data being read. */
							uint32_t ulSectorNumber,			/* Sector from which to start reading data. */
							uint32_t ulSectorCount,				/* Number of sectors to read. */
							FF_Disk_t *pxDisk );				/* Describes the disk being read from. */

static int32_t prvSpansionSPI_FFWrite( uint8_t *pucSource,		/* Source of data to be written. */
							uint32_t ulSectorNumber,			/* The first sector being written to. */
							uint32_t ulSectorCount,				/* The number of sectors to write. */
							FF_Disk_t *pxDisk );				/* Describes the disk being written to. */

/* Cache init, flush, read and write functions. */
static void prvSpansionSPI_InitCache(cache_entry_t * xCache, cache_size_t xSize);
static void prvSpansionSPI_FlushCache(cache_entry_t * xCache, cache_size_t xSize);
static cache_index_t prvSpansionSPI_ReadCache(cache_entry_t * pxCache, cache_size_t xCacheSize, uint32_t xSpiAddress);
static cache_index_t prvSpansionSPI_WriteCache(cache_entry_t * pxCache, cache_size_t xCacheSize, uint8_t * pucSource, uint32_t xSpiAddress);
static cache_stamp_t prvSpansionSPI_TimeStamp(void);

/* SPI sector read, write and erase functions. */
static void prvSpansionSPI_SectorRead(uint8_t * pucDestination, uint32_t xSpiAddress);
static void prvSpansionSPI_SectorWrite(uint8_t *pucSource, uint32_t xSpiAddress);
static void prvSpansionSPI_SectorErase(uint32_t xSpiAddress);

/* Middle level functions. */
static uint8_t prvSpansionSPI_IsBusy(void);
static void prvSpansionSPI_WriteEnable(void);
static void prvSpansionSPI_WriteDisable(void);
static void prvSpansionSPI_ChipErase(void);
static FF_Error_t prvPartitionAndFormatDisk( FF_Disk_t *pxDisk );

/* Low level functions. */
static uint8_t xSpiTransferByte(uint8_t xTX_Data);

#endif /* FREERTOS_PLUS_FAT_PORTABLE_HERCULES_S25FL1XXK_MA_SPI_FLASH_CACHE_H_ */
