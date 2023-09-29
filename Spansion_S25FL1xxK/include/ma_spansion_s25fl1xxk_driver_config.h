/**
 * @file ma_spansion_s25fl1xxk_driver_config.h
 *
 * @brief FreeRTOS+FAT driver for Spansion S25FL1xxk SPI flash devices.
 * @author Lovas Szilárd <lovas.szilard@gmail.com>
 * @Version: 0.1
 * @note: initial version
 * @Date: 2017-03-26 8:00
 */

#ifndef FREERTOS_PLUS_FAT_PORTABLE_SPANSION_S25FL1XXK_INCLUDE_MA_SPANSION_S25FL1XXK_DRIVER_CONFIG_H_
#define FREERTOS_PLUS_FAT_PORTABLE_SPANSION_S25FL1XXK_INCLUDE_MA_SPANSION_S25FL1XXK_DRIVER_CONFIG_H_

/* Cache related defs, typedefs and data structures. */
#define cacheSPI_CACHE_SIZE (2)
#define cacheSPI_CACHE_FAT_RESERVED_SIZE (1)

#if ( cacheSPI_CACHE_SIZE < 1 || cacheSPI_CACHE_SIZE <= cacheSPI_CACHE_FAT_RESERVED_SIZE)
#error "cacheSPI_CACHE_SIZE must be greater than 1 and cacheSPI_CACHE_FAT_RESERVED_SIZE less than cacheSPI_CACHE_SIZE."
#endif

#define cacheSPI_SECTOR_SIZE (4096)
#define cacheLINE_SIZE (1 * cacheSPI_SECTOR_SIZE)

#define spansionFAT_SECTOR_SIZE				512
#define spansionSPI_SECTOR_SIZE				4096
#define spansionSPI_PAGE_SIZE				256
#define spansionSPI_SECTOR_COUNT			16384	/* 8 MByte */
#define spansionSPI_IOMANAGER_CACHE_SIZE	(20 * spansionFAT_SECTOR_SIZE)
#define spansionSPI_PARTITION_NUMBER		0
#define spansionSPI_SIGNATURE				0xABBA1234
#define spansionSPI_HIDDEN_SECTOR_COUNT		8
#define spansionSPI_PRIMARY_PARTITIONS		1


//#define traceSPI_FLASH_FFREAD_START(Address, FATSecCnt, Time)		vLoggingPrintf("r,%x,%d,%llu", Address, FATSecCnt, Time)
//#define traceSPI_FLASH_FFREAD_END(Address, FATSecCnt, Time)			vLoggingPrintf(",%llu\r\n", Time)
//#define traceSPI_FLASH_FFREAD_CHECK_START(Address, FATSecCnt, Time)	vLoggingPrintf("c,%x,%d,%llu", Address, FATSecCnt, Time)
//#define traceSPI_FLASH_FFREAD_CHECK_END(Address, FATSecCnt, Time)	vLoggingPrintf(",%llu\r\n", Time)
//#define traceSPI_FLASH_FFWRITE_START(Address, FATSecCnt, Time)
//#define traceSPI_FLASH_FFWRITE_END(Address, FATSecCnt, Time)
//#define traceSPI_FLASH_WRITE_SECTOR_START(Address, FATSecCnt, Time)	vLoggingPrintf("w,%x,%d,%llu", Address, FATSecCnt, Time)
//#define traceSPI_FLASH_WRITE_SECTOR_END(Address, FATSecCnt, Time)	vLoggingPrintf(",%llu\r\n", Time)
//#define traceSPI_FLASH_ERASE_SECTOR_START(Address, FATSecCnt, Time)	vLoggingPrintf("e,%x,%d,%llu", Address, FATSecCnt, Time)
//#define traceSPI_FLASH_ERASE_SECTOR_END(Address, FATSecCnt, Time)	vLoggingPrintf(",%llu\r\n", Time)

#define traceSPI_FLASH_FFREAD_START(Address, FATSecCnt, Time)
#define traceSPI_FLASH_FFREAD_END(Address, FATSecCnt, Time)
#define traceSPI_FLASH_FFREAD_CHECK_START(Address, FATSecCnt, Time)
#define traceSPI_FLASH_FFREAD_CHECK_END(Address, FATSecCnt, Time)
#define traceSPI_FLASH_FFWRITE_START(Address, FATSecCnt, Time)
#define traceSPI_FLASH_FFWRITE_END(Address, FATSecCnt, Time)
#define traceSPI_FLASH_WRITE_SECTOR_START(Address, FATSecCnt, Time)
#define traceSPI_FLASH_WRITE_SECTOR_END(Address, FATSecCnt, Time)
#define traceSPI_FLASH_ERASE_SECTOR_START(Address, FATSecCnt, Time)
#define traceSPI_FLASH_ERASE_SECTOR_END(Address, FATSecCnt, Time)


//#define traceSPI_FLASH_FFREAD_START(Address, FATSecCnt, Time)		vLoggingPrintf("RF,%x,%d,%llu,%llu\r\n", Address, FATSecCnt, Time, Time)
//#define traceSPI_FLASH_FFREAD_END(Address, FATSecCnt, Time)
//#define traceSPI_FLASH_FFWRITE_START(Address, FATSecCnt, Time)		vLoggingPrintf("WF,%x,%d,%llu,%llu\r\n", Address, FATSecCnt, Time, Time)
//#define traceSPI_FLASH_FFWRITE_END(Address, FATSecCnt, Time)

//#define traceSPI_FLASH_READ_SECTOR_START(Address, FATSecCnt, Time)	vLoggingPrintf("r,%x,%d,%llu", Address, FATSecCnt, Time)
//#define traceSPI_FLASH_READ_SECTOR_END(Address, FATSecCnt, Time)	vLoggingPrintf(",%llu\r\n", Time)
//#define traceSPI_FLASH_WRITE_SECTOR_START(Address, FATSecCnt, Time)	vLoggingPrintf("w,%x,%d,%llu", Address, FATSecCnt, Time)
//#define traceSPI_FLASH_WRITE_SECTOR_END(Address, FATSecCnt, Time)	vLoggingPrintf(",%llu\r\n", Time)
//#define traceSPI_FLASH_ERASE_SECTOR_START(Address, FATSecCnt, Time)	vLoggingPrintf("e,%x,%d,%llu", Address, FATSecCnt, Time)
//#define traceSPI_FLASH_ERASE_SECTOR_END(Address, FATSecCnt, Time)	vLoggingPrintf(",%llu\r\n", Time)

//#define traceSPI_FLASH_FFREAD_START(Address, FATSecCnt, Time)
#define traceSPI_FLASH_FFREAD_END(Address, FATSecCnt, Time)
//#define traceSPI_FLASH_FFWRITE_START(Address, FATSecCnt, Time)
#define traceSPI_FLASH_FFWRITE_END(Address, FATSecCnt, Time)

//#define traceSPI_FLASH_FFREAD_START2(Area, Address, FATSecCnt)
//#define traceSPI_FLASH_FFWRITE_START2(Area, Address, FATSecCnt)

#define traceSPI_FLASH_FFREAD_START2(Area, Address, FATSecCnt)		vLoggingPrintf("R,%c,%d,%d,\r\n", Area, Address, FATSecCnt)
#define traceSPI_FLASH_FFWRITE_START2(Area, Address, FATSecCnt)		vLoggingPrintf("W,%c,%d,%d,\r\n", Area, Address, FATSecCnt)


#define traceSPI_FLASH_READ_SECTOR_START(Address, FATSecCnt, Time)
#define traceSPI_FLASH_READ_SECTOR_END(Address, FATSecCnt, Time)
#define traceSPI_FLASH_WRITE_SECTOR_START(Address, FATSecCnt, Time)
#define traceSPI_FLASH_WRITE_SECTOR_END(Address, FATSecCnt, Time)
#define traceSPI_FLASH_ERASE_SECTOR_START(Address, FATSecCnt, Time)
#define traceSPI_FLASH_ERASE_SECTOR_END(Address, FATSecCnt, Time)



#endif /* FREERTOS_PLUS_FAT_PORTABLE_SPANSION_S25FL1XXK_INCLUDE_MA_SPANSION_S25FL1XXK_DRIVER_CONFIG_H_ */
