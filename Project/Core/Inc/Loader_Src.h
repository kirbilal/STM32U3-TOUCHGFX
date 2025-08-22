/*
 * Loader_Src.h
 *
 *  Created on: Aug 22, 2025
 *      Author: bilalk
 */

#ifndef INC_LOADER_SRC_H_
#define INC_LOADER_SRC_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32u3xx_hal.h"
#include "stm32u3xx_hal_spi.h"
#include "mx25l6433f.h"
#include <string.h>
#include <stdio.h>

#define TIMEOUT 5000U
#define KeepInCompilation __attribute__((used))

int Init (void);
KeepInCompilation int MassErase (uint32_t Parallelism );
KeepInCompilation int Write (uint32_t Address, uint32_t Size, uint8_t* buffer);
KeepInCompilation int SectorErase (uint32_t EraseStartAddress ,uint32_t EraseEndAddress);
KeepInCompilation uint64_t Verify (uint32_t MemoryAddr, uint32_t RAMBufferAddr, uint32_t Size, uint32_t missalignement);
KeepInCompilation HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority);
int SystemClockInit(void);
void HAL_MspInit(void);
void MX_SPI2_DeInit(void);


#endif /* INC_LOADER_SRC_H_ */
