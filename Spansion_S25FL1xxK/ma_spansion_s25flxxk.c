/**
 * @file ma_spansion_s25fl1xxk.c
 *
 * @brief FreeRTOS+FAT driver for Spansion S25FL1xxk SPI flash devices.
 * @author Lovas Szilárd <lovas.szilard@gmail.com>
 * @version: 0.3
 * @date: 2017-03-26 8:00
 * - initial version
 * @Date: 2017-03-27 8:00
 * - 0.2
 * + Code reorganization
 * @date: 2017-05-03 8:00
 * - 0.3
 * + function BaseType_t xSpansionSPI_IsChipEraseInProgress(void) added.
 */

#include "ma_spansion_s25fl1xxk.h"

/* Put the (cache) logic and the low level parts into single compilation unit. */
#include "ma_spansion_s25fl1xxk_cache.inc"
#include "portable/ma_hercules_bit_banging.inc"
#include "ma_spansion_s25fl1xxk_transfer.inc"

static FF_Error_t prvPartitionAndFormatDisk(FF_Disk_t *pxDisk);
static FF_Error_t FF_SPIDiskDestroy(char *pcName);
static void prvSpansionSPI_LowLevelInit(void);

/**
 * @fn FF_Disk_t *FF_SPIDiskInit(char *pcName, uint8_t ucNeedFormat)
 * @brief Initializes and mount the SPI disk
 * @param pcName path
 * @param ucNeedFormat (pdTRUE if disk format is needed)
 * @return pointer to the FF_Disk_t structure
 */
FF_Disk_t *FF_SPIDiskInit(char *pcName, uint8_t ucNeedFormat)
{
	FF_Error_t xError;
	FF_Disk_t *pxDisk = NULL;
	FF_CreationParameters_t xParameters;

	prvSpansionSPI_LowLevelInit();

	/* Attempt to allocated the FF_Disk_t structure. */
    pxDisk = ( FF_Disk_t * ) pvPortMalloc( sizeof( FF_Disk_t ) );

    if( pxDisk != NULL )
    {
        /* It is advisable to clear the entire structure to zero after it has been
        allocated - that way the media driver will be compatible with future
        FreeRTOS+FAT versions, in which the FF_Disk_t structure may include
        additional members. */
        memset( pxDisk, '\0', sizeof( FF_Disk_t ) );

        /* The pvTag member of the FF_Disk_t structure allows the structure to be
        extended to also include media specific parameters. */
        pxDisk->pvTag = ( void * )NULL;

        /* The signature is used by the disk read and disk write functions to
        ensure the disk being accessed is an SPI disk. */
        pxDisk->ulSignature = spansionSPI_SIGNATURE;

        /* The number of sectors is recorded for bounds checking in the read and
        write functions. */
        pxDisk->ulNumberOfSectors = spansionSPI_SECTOR_COUNT;

        /* Create the IO manager that will be used to control the SPI disk -
        the FF_CreationParameters_t structure completed with the required
        parameters, then passed into the FF_CreateIOManager() function. */
        memset (&xParameters, '\0', sizeof xParameters);
        xParameters.pucCacheMemory = NULL;
        xParameters.ulMemorySize = spansionSPI_IOMANAGER_CACHE_SIZE;
        xParameters.ulSectorSize = spansionFAT_SECTOR_SIZE;
        xParameters.fnWriteBlocks = prvSpansionSPI_FFWrite;
        xParameters.fnReadBlocks = prvSpansionSPI_FFRead;
        xParameters.pxDisk = pxDisk;

        /* The Spansion SPI driver is not re-entrant */
        xParameters.pvSemaphore = ( void * ) xSemaphoreCreateRecursiveMutex();
        xParameters.xBlockDeviceIsReentrant = pdFALSE;

        pxDisk->pxIOManager = FF_CreateIOManger( &xParameters, &xError );

        if( ( pxDisk->pxIOManager != NULL ) && ( FF_isERR( xError ) == pdFALSE ) )
        {
            /* Record that the SPI disk has been initialised. */
            pxDisk->xStatus.bIsInitialised = pdTRUE;

            if(ucNeedFormat)
            {
            	xError = prvPartitionAndFormatDisk(pxDisk);
            }

            if( FF_isERR( xError ) == pdFALSE )
            {
                /* Record the partition number the FF_Disk_t structure is, then
                mount the partition. */
                pxDisk->xStatus.bPartitionNumber = spansionSPI_PARTITION_NUMBER;

                /* Mount the partition. */
                xError = FF_Mount( pxDisk, spansionSPI_PARTITION_NUMBER );
            }

            if( FF_isERR( xError ) == pdFALSE )
            {
                /* The partition mounted successfully, add it to the virtual
                file system - where it will appear as a directory off the file
                system's root directory. */
                FF_FS_Add( pcName, pxDisk );
            }
        }
        else
        {
            /* The disk structure was allocated, but the disk's IO manager could
            not be allocated, so free the disk again. */
    		pxDisk->ulSignature = 0;
    		pxDisk->xStatus.bIsInitialised = 0;
    		if( pxDisk->pxIOManager != NULL )
    		{
    			FF_DeleteIOManager( pxDisk->pxIOManager );
    		}
    		vPortFree( pxDisk );
        }
    }

    return pxDisk;
}

/**
 * @fn static BaseType_t FF_SPIDiskDestroy(char *pcName)
 * @brief unmount and delete disk on pcName path
 * @param pcName path
 * @return FF_Error_t error code
 */
static FF_Error_t FF_SPIDiskDestroy(char *pcName)
{
	FF_Error_t xError;
	FF_Disk_t *pxDisk = NULL;
	FF_SubSystem_t xSubsystemEntry;
	int xNameLen = (int) strlen(pcName), i;

	/* Search for disk named pcName */
	for(i=0; i<FF_FS_Count(); i++)
	{
		FF_FS_Get(i, &xSubsystemEntry);
		if((xSubsystemEntry.xPathlen == xNameLen) && (memcmp( xSubsystemEntry.pcPath, pcName, (size_t)xNameLen) == 0))
		{
			pxDisk = xSubsystemEntry.pxManager->xBlkDevice.pxDisk;
			break;
		}
	}

	/* Unmount the disk. */
	if(pxDisk != NULL)
	{
		xError = FF_Unmount(pxDisk);
	}
	else
	{
		xError = FF_ERR_NULL_POINTER;
	}

	/* Delete the IOManager. */
	if(xError == FF_ERR_NONE)
	{
		if(pxDisk->pxIOManager != NULL)
		{
			FF_DeleteIOManager(pxDisk->pxIOManager);
		}
		vPortFree( pxDisk );
	}
    return xError;
}

/**
 * @fn static FF_Error_t prvPartitionAndFormatDisk( FF_Disk_t *pxDisk )
 * @brief formats and creates partition on disk pxDisk
 * @param pxDisk
 * @return FF_Error_t error code
 */
static FF_Error_t prvPartitionAndFormatDisk( FF_Disk_t *pxDisk )
{
	FF_PartitionParameters_t xPartition;
	FF_Error_t xError;

	/* Create a single partition that fills all available space on the disk. */
	memset( &xPartition, '\0', sizeof( xPartition ) );
	xPartition.ulSectorCount = pxDisk->ulNumberOfSectors;
	xPartition.ulHiddenSectors = spansionSPI_HIDDEN_SECTOR_COUNT;
	xPartition.xPrimaryCount = spansionSPI_PRIMARY_PARTITIONS;
	xPartition.eSizeType = eSizeIsQuota;

	/* Partition the disk */
	xError = FF_Partition( pxDisk, &xPartition );
	FF_PRINTF( "FF_Partition: %s\n", ( const char * ) FF_GetErrMessage( xError ) );

	if( FF_isERR( xError ) == pdFALSE )
	{
		/* Format the partition. */
		xError = FF_Format( pxDisk, spansionSPI_PARTITION_NUMBER, pdTRUE, pdTRUE );
		FF_PRINTF( "FF_SPIDiskInit: FF_Format: %s\n", ( const char * ) FF_GetErrMessage( xError ) );
	}

	return xError;
}

void vSpansionSPI_PartitionAndFormatDisk(char *pcName)
{
#if(0)
	if(FF_SPIDiskDestroy(pcName) != (FF_ERR_IOMAN_ACTIVE_HANDLES | FF_UNMOUNT))
	{
		FF_SPIDiskInit(pcName, pdTRUE);
	}
#else
	FF_SPIDiskDestroy(pcName);
	FF_SPIDiskInit(pcName, pdTRUE);
#endif
}

BaseType_t xSpansionSPI_SyncCache(FF_Disk_t *pxDisk, cache_stamp_t xOlderThan)
{
	BaseType_t xReturn = pdFALSE;

	if(pxDisk != NULL && pxDisk->pxIOManager->pvSemaphore != NULL)
	{
		xReturn = xSemaphoreTakeRecursive((SemaphoreHandle_t)pxDisk->pxIOManager->pvSemaphore, 1);
		if(pdTRUE == xReturn)
		{
			prvSpansionSPI_SyncCache(xSpiCache, cacheSPI_CACHE_SIZE, xOlderThan);
			xSemaphoreGiveRecursive((SemaphoreHandle_t)pxDisk->pxIOManager->pvSemaphore);
		}
	}
	return(xReturn);
}

/**
 * @fn BaseType_t xSpansionSPI_IsChipEraseInProgress(void)
 * @brief Check if the chip erase is being in progress.
 * @return pdTRUE in case of device is busy
 */
inline BaseType_t xSpansionSPI_IsChipEraseInProgress(void)
{
	return(prvSpansionSPI_IsChipEraseInProgress());
}

/**
 * @fn void vSpansionSPI_ChipErase(void)
 * @brief Public wrapper for prvSpansionSPI_ChipErase() function.
 */
void vSpansionSPI_ChipErase(void)
{
	prvSpansionSPI_ChipErase();
}

static void prvSpansionSPI_LowLevelInit(void)
{

	prvSpansionSPI_InitCache(xSpiCache, cacheSPI_CACHE_SIZE);
	prvSpansionSPI_QuadEnable();
}
