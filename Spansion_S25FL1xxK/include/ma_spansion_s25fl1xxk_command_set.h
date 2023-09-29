/**
 * @file ma_spansion_s25fl1xxk_command_set.h
 * @brief Spansion S25FL1xxK SPI flash EEprom FreeRTOS+FAT driver.
 * @author Lovas Szilárd <lovas.szilard@gmail.com>
 * @Version: 0.2
 * @Date: 2017-02-22 14:00
 * - 0.1 - initial version
 * @Date: 2017-03-26 14:00
 * - 0.2 - Code reorganization
 */

#ifndef FREERTOS_PLUS_FAT_PORTABLE_HERCULES_S25FL1XXK_MA_SPI_S25FL1XXK_COMMAND_SET_H_
#define FREERTOS_PLUS_FAT_PORTABLE_HERCULES_S25FL1XXK_MA_SPI_S25FL1XXK_COMMAND_SET_H_

/* Spansion S25FL1xxK serial flash command set. */

/* Configuration, Status, Erase, Program Commands */
#define spansionReadStatusRegister1  (uint8_t)0x05
#define spansionReadStatusRegister2 (uint8_t)0x35
#define spansionReadStatusRegister3 (uint8_t)0x33
#define spansionWriteEnable (uint8_t)0x06
#define spansionWriteEnableForVolatileStatusRegister (uint8_t)0x50
#define spansionWriteDisable (uint8_t)0x04
#define spansionWriteStatusRegisters (uint8_t)0x01
#define spansionSetBurstWithWrap (uint8_t)0x77
#define spansionSetBlockPointerProtection (uint8_t)0x39		/* (S25FL132K / S25FL164K) */
#define spansionPageProgram (uint8_t)0x02
#define spansionSectorErase (uint8_t)0x20					/* (4 kB) */
#define spansionBlockErase (uint8_t)0xD8					/* (64 kB) */
#define spansionChipErase (uint8_t)0xc7						/*.. or 0x60 */
#define spansionEraseProgramSuspend (uint8_t)0x75
#define spansionEraseProgramResume (uint8_t)0x7a

/* Read Commands */
#define spansionReadData (uint8_t)0x03
#define spansionFastRead (uint8_t)0x0b
#define spansionFastReadDualOutput (uint8_t)0x3b
#define spansionFastReadQuadOutput (uint8_t)0x6b
#define spansionFastReadDualIO (uint8_t)0xbb
#define spansionFastReadQuadIO (uint8_t)0xeb
#define xCmdContinuousReadModeReset (uint8_t)0xff

/* Reset Commands */
#define spansionSoftwareResetEnable (uint8_t)0x66
#define spansionSoftwareReset (uint8_t)0x99

#endif /* FREERTOS_PLUS_FAT_PORTABLE_HERCULES_S25FL1XXK_MA_SPI_S25FL1XXK_COMMAND_SET_H_ */
