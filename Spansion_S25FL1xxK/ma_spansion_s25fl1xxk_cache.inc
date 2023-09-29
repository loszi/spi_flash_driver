/**
 * @file ma_spansion_s25fl1xxk_cache.inc
 *
 * @brief WriteBack cache API for SPI flash devices.
 * @author Lovas Szilárd <lovas.szilard@gmail.com>
 * @Version: 0.3
 * @Date: 2017-02-27 11:00
 * - 0.1 - initial version
 * @Date: 2017-03-15 17:00
 * - 0.2
 * + Dedicated cache for FAT area sectors (#define cacheSPI_CACHE_FAT_RESERVED_SIZE)
 * + Improved cache algorithm (check validity during replace), "clean write" 83.2 KiB/sec, "dirty write" 23.6 KiB/sec @ 20 KiB cache
 * + Significant performance gain at dirty write with same content over non-cached version  ~ 208 KiB/sec (5MHz spi clk)
 * @Date: 2017-03-27 8:00
 * - 0.3
 * + Code reorganization (separated low level functions).
 */


#include "FreeRTOS.h"
#include "os_task.h"
#include "os_semphr.h"
#include "os_portmacro.h"

/* FreeRTOS+FAT includes. */
#include "ff_headers.h"
#include "ff_sys.h"

#include "sys_main.h"
#include "rti_runtimestats.h"

#include "ma_spansion_s25fl1xxk_driver_config.h"
#include "ma_spansion_s25fl1xxk_cache.h"

/* SPI Cache */
static cache_entry_t xSpiCache[cacheSPI_CACHE_SIZE];


/* From Hein */
BaseType_t xIsFatSector( FF_IOManager_t *pxIOManager, uint32_t ulSectorNr )
{
uint32_t ulFirst;
uint32_t ulLast;
BaseType_t xResult;

    if( pxIOManager != NULL )
    {
        ulFirst = pxIOManager->xPartition.ulFATBeginLBA;
        ulLast = ulFirst + pxIOManager->xPartition.ulSectorsPerFAT * ( uint32_t )pxIOManager->xPartition.ucNumFATS;
        xResult = ulSectorNr >= ulFirst && ulSectorNr < ulLast;
    }
    else
    {
        xResult = -1;
    }
    return xResult;
}

/**
 * @fn static void prvSpansionSPI_InitCache(cache_entry_t *pxCache, cache_size_t xSize)
 * @brief initializes the cache entries.
 * @param cache_entry_t *pxCache pointer to the cache.
 * @param cache_size_t xSize number of the cache entries.
 */
static void prvSpansionSPI_InitCache(cache_entry_t *pxCache, cache_size_t xSize)
{
	cache_size_t i;

	/* Initializes cache entries. */
	for(i = 0; i < xSize; i++)
	{
		pxCache[i].State = INVALID;
		pxCache[i].Tag = 0;
		pxCache[i].Stamp = (cache_stamp_t)0;
		memset(pxCache[i].Line, 0, cacheLINE_SIZE);
	}
}

/**
 * @fn static void prvSpansionSPI_SyncCache(cache_entry_t *pxCache, cache_size_t xSize, cache_stamp_t xOlderThan)
 * @brief synchronizes the cache content with the Spansion SPI Flash media.
 * @param cache_entry_t *pxCache pointer to the cache.
 * @param cache_size_t xSize number of the cache entries.
 * @param cache_stamp_t xOlderThan sync cache entries with stamp older than xOlderThan.
 */
static void prvSpansionSPI_SyncCache(cache_entry_t *pxCache, cache_size_t xSize, cache_stamp_t xOlderThan)
{
	cache_size_t i;

	cache_stamp_t xNow = prvSpansionSPI_TimeStamp();

	/* Sync all the old cache entries. */
	for(i=0; i<xSize; i++)
	{
		if((pxCache[i].Stamp < xNow - xOlderThan) || (xOlderThan == 0))
		{
			switch(pxCache[i].State)
			{
			case INVALID:
			case VALID:
				break;
			case INCOMPATIBLE:
				/* Erase */
				prvSpansionSPI_SectorErase(pxCache[i].Tag);
			case MODIFIED:
				/* Write */
				prvSpansionSPI_SectorWrite(pxCache[i].Line, pxCache[i].Tag);
				pxCache[i].State = VALID;
				break;
			}
		}
	}
}

/**
 * @fn static int32_t prvSpansionSPI_FFRead(uint8_t *pucSource, uint32_t ulSectorNumber, uint32_t ulSectorCount, FF_Disk_t *pxDisk)
 * @brief Read sectors from Spansion SPI Flash media.
 * @param pucDestination Destination of data to be read.
 * @param ulSectorNumber The first sector being written to.
 * @param ulSectorCount The number of sectors to read.
 * @param pxDisk Describes the disk being read from.
 * @return Error code
 */
static int32_t prvSpansionSPI_FFRead( uint8_t *pucDestination,	/* Destination for data being read. */
							uint32_t ulSectorNumber,	/* Sector from which to start reading data. */
							uint32_t ulSectorCount,		/* Number of sectors to read. */
							FF_Disk_t *pxDisk )			/* Describes the disk being read from. */
	{
	uint32_t xSpiAddress = (ulSectorNumber * spansionFAT_SECTOR_SIZE) & 0x00ffffff;
	uint32_t i;
	cache_index_t xCacheIndex;

	if(prvSpansionSPI_IsBusy())
		{
		return(FF_ERR_DRIVER_BUSY);
		}

	/* Trace macros. */
	if(xIsFatSector(pxDisk->pxIOManager, ulSectorNumber))
		{
		traceSPI_FLASH_FFREAD_START2('F', ulSectorNumber, ulSectorCount);
		}
	else
		{
		traceSPI_FLASH_FFREAD_START2('D', ulSectorNumber, ulSectorCount);
		}

	for(i = 0; i < ulSectorCount; i++)
		{
#if(cacheSPI_CACHE_FAT_RESERVED_SIZE > 0)
		/* Check if the current sector belongs to the FAT area. */
		if(xIsFatSector(pxDisk->pxIOManager, ulSectorNumber + i))
			{
			/* FAT area. */
			xCacheIndex = prvSpansionSPI_ReadCache(xSpiCache, cacheSPI_CACHE_FAT_RESERVED_SIZE, xSpiAddress & 0x00fff000);
			memcpy(pucDestination, &xSpiCache[xCacheIndex].Line[xSpiAddress & 0x00000fff], spansionFAT_SECTOR_SIZE);
			}
		else
			{
			/* Data area. */
			xCacheIndex = prvSpansionSPI_ReadCache(&xSpiCache[cacheSPI_CACHE_FAT_RESERVED_SIZE], cacheSPI_CACHE_SIZE - cacheSPI_CACHE_FAT_RESERVED_SIZE, xSpiAddress & 0x00fff000);
			xCacheIndex += cacheSPI_CACHE_FAT_RESERVED_SIZE;
			memcpy(pucDestination, &xSpiCache[xCacheIndex].Line[xSpiAddress & 0x00000fff], spansionFAT_SECTOR_SIZE);
			}
#else
		xCacheIndex = prvSpansionSPI_ReadCache(xSpiCache, cacheSPI_CACHE_SIZE, xSpiAddress & 0x00fff000);
		memcpy(pucDestination, &xSpiCache[xCacheIndex].Line[xSpiAddress & 0x00000fff], spansionFAT_SECTOR_SIZE);
#endif
		xSpiAddress += spansionFAT_SECTOR_SIZE;
		pucDestination += spansionFAT_SECTOR_SIZE;
		}

	/* Trace macro. */
	traceSPI_FLASH_FFREAD_END(ulSectorNumber, ulSectorCount, xGetHighResolutionTime());

	return(FF_ERR_NONE);
	}

/**
 * @fn static int32_t prvSpansionSPI_FFWrite(uint8_t *pucSource, uint32_t ulSectorNumber, uint32_t ulSectorCount, FF_Disk_t *pxDisk)
 * @brief Write sectors to Spansion SPI Flash media.
 * @param pucSource Source of data to be written.
 * @param ulSectorNumber The first sector being written to.
 * @param ulSectorCount The number of sectors to write.
 * @param pxDisk Describes the disk being written to.
 * @return Error code
 */
static int32_t prvSpansionSPI_FFWrite( uint8_t *pucSource,		/* Source of data to be written. */
							uint32_t ulSectorNumber,	/* The first sector being written to. */
							uint32_t ulSectorCount,		/* The number of sectors to write. */
							FF_Disk_t *pxDisk )			/* Describes the disk being written to. */
	{
	uint32_t xSpiAddress = (ulSectorNumber * spansionFAT_SECTOR_SIZE) & 0x00ffffff;
	uint32_t i;

	/* Trace macros. */
	if(xIsFatSector(pxDisk->pxIOManager, ulSectorNumber))
		{
		traceSPI_FLASH_FFWRITE_START2('F', ulSectorNumber, ulSectorCount);
		}
	else
		{
		traceSPI_FLASH_FFWRITE_START2('D', ulSectorNumber, ulSectorCount);
		}

	if(prvSpansionSPI_IsBusy())
		{
		return(FF_ERR_DRIVER_BUSY);
		}

	for(i = 0; i < ulSectorCount; i++)
		{

#if(cacheSPI_CACHE_FAT_RESERVED_SIZE > 0)
		/* Check if the current sector belongs to the FAT area. */
		if(xIsFatSector(pxDisk->pxIOManager, ulSectorNumber + i))
			{
			/* FAT area. */
			prvSpansionSPI_WriteCache(xSpiCache, cacheSPI_CACHE_FAT_RESERVED_SIZE, pucSource, xSpiAddress);
			}
		else
			{
			/* Data area. */
			prvSpansionSPI_WriteCache(&xSpiCache[cacheSPI_CACHE_FAT_RESERVED_SIZE], cacheSPI_CACHE_SIZE - cacheSPI_CACHE_FAT_RESERVED_SIZE, pucSource, xSpiAddress);
			}

#else
		prvSpansionSPI_WriteCache(xSpiCache, cacheSPI_CACHE_SIZE, pucSource, xSpiAddress);
#endif
		xSpiAddress += spansionFAT_SECTOR_SIZE;
		pucSource += spansionFAT_SECTOR_SIZE;
		}

	/* Trace macro. */
	traceSPI_FLASH_FFWRITE_END(ulSectorNumber, ulSectorCount, xGetHighResolutionTime());

	return FF_ERR_NONE;
	}

/**
 * @fn static cache_index_t prvSpansionSPI_ReadCache(cache_entry_t * pxCache, cache_size_t xCacheSize, uint32_t xSpiAddress)
 * @brief Read an SPI sector (4096 byte) trough the Cache.
 * @param pxCache Pointer to the cache.
 * @param xCacheSize Size of the cache.
 * @param xSpiAddress SPI address.
 * @return Index of the cache entry, that contains the data.
 */
static cache_index_t prvSpansionSPI_ReadCache(cache_entry_t * pxCache, cache_size_t xCacheSize, uint32_t xSpiAddress)
{
	int i;
	cache_stamp_t xLeastRecentUsedStamp = (cache_stamp_t)(-1);
	cache_index_t xLeastRecentUsedIndex = 0;
	cache_index_t xInvalidEntryIndex = -1;

	/* Searching for the xSpiAddress tag and the least recent used entry in the cache.
	   If exist - invalid entry has priority over the least recent used entry. */

	for(i=0; i < xCacheSize; i++)
	{
		if(pxCache[i].State != INVALID)
		{
			if(pxCache[i].Tag == xSpiAddress)
			{
				/* Update the time stamp. */
				pxCache[i].Stamp = prvSpansionSPI_TimeStamp();

				return(i);
			}
			if(pxCache[i].Stamp < xLeastRecentUsedStamp)
			{
				xLeastRecentUsedStamp = pxCache[i].Stamp;
				xLeastRecentUsedIndex = i;
			}
		}
		else
		{
			/* Save the first invalid entry index. */
			if(xInvalidEntryIndex < 0)xInvalidEntryIndex = i;
		}
	}

	if(xInvalidEntryIndex >= 0)xLeastRecentUsedIndex = xInvalidEntryIndex;

	/* xSpiAddress entry have't found in the cache - we have to fetch it now. */
	switch(pxCache[xLeastRecentUsedIndex].State)
	{
		case INVALID:
		case VALID:
			break;
		case INCOMPATIBLE:
			/* Erase. */
			prvSpansionSPI_SectorErase(pxCache[xLeastRecentUsedIndex].Tag);
		case MODIFIED:
			/* Write. */
			prvSpansionSPI_SectorWrite(pxCache[xLeastRecentUsedIndex].Line, pxCache[xLeastRecentUsedIndex].Tag);
			break;
	}

	/* Read */
	prvSpansionSPI_SectorRead(pxCache[xLeastRecentUsedIndex].Line, xSpiAddress);
	pxCache[xLeastRecentUsedIndex].Tag = xSpiAddress;
	pxCache[xLeastRecentUsedIndex].Stamp = prvSpansionSPI_TimeStamp();
	pxCache[xLeastRecentUsedIndex].State = VALID;

	return(xLeastRecentUsedIndex);
}

/**
 * @fn static cache_index_t prvSpansionSPI_WriteCache(cache_entry_t * pxCache, cache_size_t xCacheSize, uint8_t * pucSource, uint32_t xSpiAddress)
 * @brief Write a FAT sector size (512 byte) area trough the Cache.
 * @param pxCache Pointer to the cache.
 * @param xCacheSize Size of the cache.
 * @param pucSource Pointer to the Source.
 * @param xSpiAddress SPI address.
 * @return Index of the cache entry, that contains the data.
 */
static cache_index_t prvSpansionSPI_WriteCache(cache_entry_t * pxCache, cache_size_t xCacheSize, uint8_t * pucSource, uint32_t xSpiAddress)
{
	uint32_t i, j;
	uint32_t xSpiSectorAddress = xSpiAddress & 0x00fff000;
	uint32_t xSubAddress = xSpiAddress & 0x00000fff;

	cache_stamp_t xLeastRecentUsedStamp = (cache_stamp_t)(-1);
	cache_index_t xLeastRecentUsedIndex = 0;
	cache_index_t xWriteHitEntryIndex = -1;
	cache_index_t xInvalidEntryIndex = -1;

	/* Searching for the xSpiAddress tag and the least recently used entry in the cache.  */
	for(i=0; i<xCacheSize; i++)
	{
		if(pxCache[i].State != INVALID)
		{
			if(pxCache[i].Tag == xSpiSectorAddress)
			{
				xWriteHitEntryIndex = i;
				break;
			}
			if(pxCache[i].Stamp < xLeastRecentUsedStamp)
			{
				xLeastRecentUsedStamp = pxCache[i].Stamp;
				xLeastRecentUsedIndex = i;
			}
		}
		else
		{
			/* Save the first invalid entry index. */
			if(xInvalidEntryIndex < 0)xInvalidEntryIndex = i;
		}
	}

	if(xInvalidEntryIndex >= 0)xLeastRecentUsedIndex = xInvalidEntryIndex;
	if(xWriteHitEntryIndex >= 0)xLeastRecentUsedIndex = xWriteHitEntryIndex;

	switch(pxCache[xLeastRecentUsedIndex].State)
	{
		/* Cache entry is INVALID: reading the SPI flash is needed. */
		case INVALID:
			/* Read */
			prvSpansionSPI_SectorRead(pxCache[xLeastRecentUsedIndex].Line, xSpiSectorAddress);
			pxCache[xLeastRecentUsedIndex].Tag = xSpiSectorAddress;
			pxCache[xLeastRecentUsedIndex].State = VALID;
			break;

		/* Cache entry is VALID: content of the SPI flash and the cache entry are same. */
		case VALID:
			/* Read if it is necessary */
			if(pxCache[xLeastRecentUsedIndex].Tag != xSpiSectorAddress)
			{
				prvSpansionSPI_SectorRead(pxCache[xLeastRecentUsedIndex].Line, xSpiSectorAddress);
				pxCache[xLeastRecentUsedIndex].Tag = xSpiSectorAddress;
				pxCache[xLeastRecentUsedIndex].State = VALID;
			}
			break;

		/* Cache entry is MODIFIED: SPI flash and the cache entry are not same, but SPI flash can be written without SPI erasing. */
		case MODIFIED:
			/* Replace if it is necessary */
			if(pxCache[xLeastRecentUsedIndex].Tag != xSpiSectorAddress)
			{
				/* Write */
				prvSpansionSPI_SectorWrite(pxCache[xLeastRecentUsedIndex].Line, pxCache[xLeastRecentUsedIndex].Tag);
				/* Read */
				prvSpansionSPI_SectorRead(pxCache[xLeastRecentUsedIndex].Line, xSpiSectorAddress);
				pxCache[xLeastRecentUsedIndex].Tag = xSpiSectorAddress;
				pxCache[xLeastRecentUsedIndex].State = VALID;
			}
			break;

		/* Cache entry is INCOMPATIBLE, SPI flash and the cache entry are not same, and SPI erasing is needed before write. */
		case INCOMPATIBLE:
			/* Replace if it is necessary */
			if(pxCache[xLeastRecentUsedIndex].Tag != xSpiSectorAddress)
			{
				/* Erase */
				prvSpansionSPI_SectorErase(pxCache[xLeastRecentUsedIndex].Tag);
				/* Write */
				prvSpansionSPI_SectorWrite(pxCache[xLeastRecentUsedIndex].Line, pxCache[xLeastRecentUsedIndex].Tag);
				/* Read */
				prvSpansionSPI_SectorRead(pxCache[xLeastRecentUsedIndex].Line, xSpiSectorAddress);
				pxCache[xLeastRecentUsedIndex].Tag = xSpiSectorAddress;
				pxCache[xLeastRecentUsedIndex].State = VALID;
			}
			break;
	}

	/* Modify */
	for(j = 0; j < spansionFAT_SECTOR_SIZE; j++)
	{
		if(pxCache[xLeastRecentUsedIndex].State == VALID && pxCache[xLeastRecentUsedIndex].Line[xSubAddress + j] != pucSource[j])
		{
			pxCache[xLeastRecentUsedIndex].State = MODIFIED;
		}
		/* NOR checking */
		if(pxCache[xLeastRecentUsedIndex].State != INCOMPATIBLE)
		{
			if(((uint8_t)(pxCache[xLeastRecentUsedIndex].Line[xSubAddress + j] | ~pucSource[j])) != 0xff)pxCache[xLeastRecentUsedIndex].State = INCOMPATIBLE;
		}
		pxCache[xLeastRecentUsedIndex].Line[xSubAddress + j] = pucSource[j];
	}
	pxCache[xLeastRecentUsedIndex].Stamp = prvSpansionSPI_TimeStamp();

	return(xLeastRecentUsedIndex);
}

inline static cache_stamp_t prvSpansionSPI_TimeStamp(void)
{
//	static cache_stamp_t xTimeStamp = 0;
//	return(++xTimeStamp);
	return(xGetHighResolutionTime());
}
