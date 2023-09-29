/**
 * @file ma_spansion_s25fl1xxk.h
 *
 * @brief FreeRTOS+FAT driver for Spansion S25FL1xxk SPI flash devices.
 * @author Lovas Szilárd <lovas.szilard@gmail.com>
 * @Version: 0.2
 * @Date: 2017-03-26 8:00
 * - initial version
 * @date: 2017-05-03 8:00
 * - 0.2
 * + function BaseType_t xSpansionSPI_IsChipEraseInProgress(void) added.
 */

/* FreeRTOS+FAT includes. */
#include "ff_headers.h"
#include "ff_sys.h"

#include "ma_spansion_s25fl1xxk_driver_config.h"

#if(cacheSPI_CACHE_SIZE > 0)
#include "ma_spansion_s25fl1xxk_cache.h"
#else
#include "ma_spansion_s25fl1xxk_noncache.h"
#endif

#ifndef FREERTOS_PLUS_FAT_PORTABLE_SPANSION_S25FL1XXK_INCLUDE_MA_SPANSION_S25FL1XXK_H_
#define FREERTOS_PLUS_FAT_PORTABLE_SPANSION_S25FL1XXK_INCLUDE_MA_SPANSION_S25FL1XXK_H_

FF_Disk_t *FF_SPIDiskInit(char *pcName, uint8_t ucNeedFormat);
BaseType_t xSpansionSPI_SyncCache(FF_Disk_t *pxDisk, cache_stamp_t xOlderThan);
void vSpansionSPI_PartitionAndFormatDisk(char *pcName);
void vSpansionSPI_ChipErase(void);
BaseType_t xSpansionSPI_IsChipEraseInProgress(void);

#endif /* FREERTOS_PLUS_FAT_PORTABLE_SPANSION_S25FL1XXK_INCLUDE_MA_SPANSION_S25FL1XXK_H_ */
