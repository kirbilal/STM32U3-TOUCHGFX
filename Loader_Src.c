#include "main.h"
#include "spi.h"
#include "gpio.h"
#include "mx25l6433f.h"
#include "MB1642BDataReader.h" // readData fonksiyonu için
#include "Loader_Src.h"
#include <string.h>

#define LOADER_START_ADDR 0x90000000
#define LOADER_OK 0x1
#define LOADER_FAIL 0x0
extern void SystemClock_Config(void);
extern void MX_ICACHE_Init(void);
extern void MX_USART1_UART_Init(void);
extern void MX_GPIO_Init(void);
extern char __bss_start__;
extern char __bss_end__;

extern volatile uint32_t uwTick;

extern UART_HandleTypeDef huart1;
/**
 * @brief  System initialization.
 * @param  None
 * @retval  1      : Operation succeeded
 * @retval  0      : Operation failed
 */
int Init(void) {

    __disable_irq();

    /* Init structs to Zero */
    char *startadd = &__bss_start__;
    uint32_t size = &__bss_end__ - &__bss_start__;
    memset(startadd, 0, size);

    /* init system */
    SystemInit();
    HAL_Init();

    /* Configure the system clock  */
    SystemClock_Config();

    /* Initialize peripherals */
    MX_ICACHE_Init();
    MX_USART1_UART_Init();
    char debug_msg[] = "Init: USART initialized\r\n";
     HAL_UART_Transmit(&huart1, (uint8_t*)debug_msg, strlen(debug_msg), 1000);

    MX_GPIO_Init();
    // Debug: USART hazır olduktan sonra mesaj gönder
    strcpy(debug_msg, "Init: GPIO deinitialized\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)debug_msg, strlen(debug_msg), 1000);

    /* QuadSPI De-Init */
    MX_SPI2_DeInit();

    // Debug mesajı
    strcpy(debug_msg, "Init: SPI2 deinitialized\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)debug_msg, strlen(debug_msg), 1000);

    /* QuadSPI Init */
    MX_SPI2_Init();

    strcpy(debug_msg, "Init: SPI2 initialized\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)debug_msg, strlen(debug_msg), 1000);

    if (MX25L6433F_Init() != MX25L6433F_OK) {
        strcpy(debug_msg, "Init: MX25L6433F init FAILED\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)debug_msg, strlen(debug_msg), 1000);
        return 0;
    }

    strcpy(debug_msg, "Init: MX25L6433F init OK\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)debug_msg, strlen(debug_msg), 1000);

    sFLASH_WaitForWriteEnd();

    strcpy(debug_msg, "Init: Flash ready\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)debug_msg, strlen(debug_msg), 1000);

    __enable_irq();

    strcpy(debug_msg, "Init: Completed successfully\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)debug_msg, strlen(debug_msg), 1000);

    return 1;
}

/**
 * @brief   Program memory.
 * @param   Address: page address
 * @param   Size   : size of data
 * @param   buffer : pointer to data buffer
 * @retval  1      : Operation succeeded
 * @retval  0      : Operation failed
 */
KeepInCompilation int Write(uint32_t Address, uint32_t Size, uint8_t *buffer) {

	__disable_irq();
    char debug_msg[80];
    sprintf(debug_msg, "*** Write CALLED Addr=0x%08lX Size=%lu ***\r\n", Address, Size);
    HAL_UART_Transmit(&huart1, (uint8_t*)debug_msg, strlen(debug_msg), 1000);

	/* QuadSPI De-Init */
//  MX_OCTOSPI1_DeInit();
	MX_SPI2_DeInit();
    strcpy(debug_msg, "*** Write SPI2 DeInit ***\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)debug_msg, strlen(debug_msg), 1000);

	/* QuadSPI Init */
//  MX_OCTOSPI1_Init();
	MX_SPI2_Init();
    strcpy(debug_msg, "*** Write SPI Init ***\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)debug_msg, strlen(debug_msg), 1000);


//  Address = Address & 0x0FFFFFFF;

	/* Writes an amount of data to the QSPI memory */
//  if(PY25Q64_Program(buffer, Size, Address) != PY25Q64_OK){
//    return 0;
//  }
	sFLASH_WriteBuffer(buffer, Address, Size);
    strcpy(debug_msg, "*** Write COMPLETED ***\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)debug_msg, strlen(debug_msg), 1000);

	__enable_irq();

	return 1;
}

/**
 * @brief 	 Full erase of the device
 * @param 	 Parallelism : 0
 * @retval  1           : Operation succeeded
 * @retval  0           : Operation failed
 */
KeepInCompilation int MassErase(uint32_t Parallelism) {
	__disable_irq();

	/* QuadSPI De-Init */
//  MX_OCTOSPI1_DeInit();
	MX_SPI2_DeInit();
    char debug_msg[] = "MassErase: SPI2 DeInitialized\r\n";
     HAL_UART_Transmit(&huart1, (uint8_t*)debug_msg, strlen(debug_msg), 1000);
	/* QuadSPI Init */
//  MX_OCTOSPI1_Init();
	MX_SPI2_Init();
    strcpy(debug_msg, "MassErase: SPI2 Initialized\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)debug_msg, strlen(debug_msg), 1000);


//  PY25Q64_AutoPollingMemReady();


//  	  /* Erase the entire QSPI memory */
//  	  if (PY25Q64_MassErase() != PY25Q64_OK){
//  	      return 0;
//  	  }
	sFLASH_EraseChip();
    strcpy(debug_msg, "MassErase: sFLASH_EraseChip OK\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)debug_msg, strlen(debug_msg), 1000);
	sFLASH_WaitForWriteEnd();
    strcpy(debug_msg, "MassErase: WaitForWriteEnd OK\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)debug_msg, strlen(debug_msg), 1000);
	__enable_irq();

	return 1;
}

/**
 * @brief   Sector erase.
 * @param   EraseStartAddress :  erase start address
 * @param   EraseEndAddress   :  erase end address
 * @retval  None
 */
KeepInCompilation int SectorErase(uint32_t EraseStartAddress,
		uint32_t EraseEndAddress) {
	__disable_irq();

//  uint32_t BlockAddr = 0;
//  EraseStartAddress &= 0x0FFFFFFF;
//  EraseEndAddress &= 0x0FFFFFFF;
//  EraseStartAddress = EraseStartAddress - EraseStartAddress % 0x10000;

	/* QuadSPI De-Init */
//  MX_OCTOSPI1_DeInit();
	MX_SPI2_DeInit();
    char debug_msg[] = "SectorErase: SPI2 DeInitialized\r\n";
     HAL_UART_Transmit(&huart1, (uint8_t*)debug_msg, strlen(debug_msg), 1000);

	/* QuadSPI Init */
//  MX_OCTOSPI1_Init();
	MX_SPI2_Init();
    strcpy(debug_msg, "SectorErase: SPI2 Initialized\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)debug_msg, strlen(debug_msg), 1000);

	EraseStartAddress = EraseStartAddress
			- EraseStartAddress % sFLASH_SPI_SECTOR_SIZE;
	while (EraseEndAddress >= EraseStartAddress) {
		sFLASH_EraseSector(EraseStartAddress);
		EraseStartAddress += sFLASH_SPI_SECTOR_SIZE;
	}
    strcpy(debug_msg, "SectorErase: OK\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)debug_msg, strlen(debug_msg), 1000);

	__enable_irq();

	return 1;
}

/**
 * Description :
 * Calculates checksum value of the memory zone
 * Inputs    :
 *      StartAddress  : Flash start address
 *      Size          : Size (in WORD)
 *      InitVal       : Initial CRC value
 * outputs   :
 *     R0             : Checksum value
 * Note: Optional for all types of device
 */
uint32_t CheckSum(uint32_t StartAddress, uint32_t Size, uint32_t InitVal) {
	__disable_irq();

	uint8_t missalignementAddress = StartAddress % 4;
	uint8_t missalignementSize = Size;
	int cnt;
	uint32_t Val;

	StartAddress -= StartAddress % 4;
	Size += (Size % 4 == 0) ? 0 : 4 - (Size % 4);

	for (cnt = 0; cnt < Size; cnt += 4) {
		Val = *(uint32_t*) StartAddress;
		if (missalignementAddress) {
			switch (missalignementAddress) {
			case 1:
				InitVal += (uint8_t) (Val >> 8 & 0xff);
				InitVal += (uint8_t) (Val >> 16 & 0xff);
				InitVal += (uint8_t) (Val >> 24 & 0xff);
				missalignementAddress -= 1;
				break;
			case 2:
				InitVal += (uint8_t) (Val >> 16 & 0xff);
				InitVal += (uint8_t) (Val >> 24 & 0xff);
				missalignementAddress -= 2;
				break;
			case 3:
				InitVal += (uint8_t) (Val >> 24 & 0xff);
				missalignementAddress -= 3;
				break;
			}
		} else if ((Size - missalignementSize) % 4 && (Size - cnt) <= 4) {
			switch (Size - missalignementSize) {
			case 1:
				InitVal += (uint8_t) Val;
				InitVal += (uint8_t) (Val >> 8 & 0xff);
				InitVal += (uint8_t) (Val >> 16 & 0xff);
				missalignementSize -= 1;
				break;
			case 2:
				InitVal += (uint8_t) Val;
				InitVal += (uint8_t) (Val >> 8 & 0xff);
				missalignementSize -= 2;
				break;
			case 3:
				InitVal += (uint8_t) Val;
				missalignementSize -= 3;
				break;
			}
		} else {
			InitVal += (uint8_t) Val;
			InitVal += (uint8_t) (Val >> 8 & 0xff);
			InitVal += (uint8_t) (Val >> 16 & 0xff);
			InitVal += (uint8_t) (Val >> 24 & 0xff);
		}
		StartAddress += 4;
	}

	__enable_irq();

	return (InitVal);

}

/**
 * Description :
 * Verify flash memory with RAM buffer and calculates checksum value of
 * the programmed memory
 * Inputs    :
 *      FlashAddr     : Flash address
 *      RAMBufferAddr : RAM buffer address
 *      Size          : Size (in WORD)
 *      InitVal       : Initial CRC value
 * outputs   :
 *     R0             : Operation failed (address of failure)
 *     R1             : Checksum value
 * Note: Optional for all types of device
 */
KeepInCompilation uint64_t Verify(uint32_t MemoryAddr, uint32_t RAMBufferAddr,
		uint32_t Size, uint32_t missalignement) {
	__disable_irq();

	uint32_t VerifiedData = 0, InitVal = 0;
	uint64_t checksum = 0;
	Size *= 4;

	checksum = CheckSum((uint32_t) MemoryAddr + (missalignement & 0xf),
			Size - ((missalignement >> 16) & 0xF), InitVal);
	while (Size > VerifiedData) {
		if (*(uint8_t*) MemoryAddr++
				!= *((uint8_t*) RAMBufferAddr + VerifiedData))
			return ((checksum << 32) + (MemoryAddr + VerifiedData));

		VerifiedData++;
	}

	__enable_irq();

	return (checksum << 32);
}

/**
 * @brief SPI2 Deinitialization Function
 * @param None
 * @retval None
 */
void MX_SPI2_DeInit(void) {
	 __HAL_RCC_SPI2_CLK_DISABLE();

	    /**SPI2 GPIO Configuration
    PC2     ------> SPI2_MISO
    PC3     ------> SPI2_MOSI
    PB13     ------> SPI2_SCK
	    */
	    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_13);
	    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_2 | GPIO_PIN_3);

	    /* SPI2 interrupt DeInit */
	    HAL_NVIC_DisableIRQ(SPI2_IRQn);
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
int SystemClockInit(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	uint32_t msiclk = 0x0U;

	__HAL_RCC_PWR_CLK_ENABLE();

	/** Enable Epod Booster
	 */
	if (HAL_RCCEx_EpodBoosterClkConfig(RCC_EPODBOOSTER_SOURCE_MSIS,
			RCC_EPODBOOSTER_DIV1) != HAL_OK) {
		return 0;
	}
	if (HAL_PWREx_EnableEpodBooster() != HAL_OK) {
		return 0;
	}

	/** Configure the main internal regulator output voltage
	 */
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1)
			!= HAL_OK) {
		return 0;
	}

	/** Set Flash latency before increasing MSIS
	 */
	__HAL_FLASH_SET_LATENCY(FLASH_LATENCY_2);

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSIS;
	RCC_OscInitStruct.MSISState = RCC_MSI_ON;
	RCC_OscInitStruct.MSISSource = RCC_MSI_RC0;
	RCC_OscInitStruct.MSISDiv = RCC_MSI_DIV1;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		return 0;
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK3;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSIS;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		return 0;
	}

	if (RCC_OscInitStruct.MSISSource == RCC_MSI_RC0) {
		msiclk = 96000000U;
	} else {
		msiclk = 24000000U;
	}

	switch (RCC_OscInitStruct.MSISDiv) {
	case RCC_MSI_DIV1:
		SystemCoreClock = msiclk;
		break;
	case RCC_MSI_DIV2:
		SystemCoreClock = msiclk / 2;
		break;
	case RCC_MSI_DIV4:
		SystemCoreClock = msiclk / 4;
		break;
	case RCC_MSI_DIV8:
		SystemCoreClock = msiclk / 8;
		break;
	default:
		SystemCoreClock = 24000000U;
	}

	return 1;
}

/**
 * @brief  MX25L6433F Flash memory initialization.
 * @param  None
 * @retval MX25L6433F_OK      : Operation succeeded
 * @retval MX25L6433F_CHIP_ERR: Operation failed
 */
MX25L6433F_STATE MX25L6433F_Init(void) 		//BK
{
	uint32_t flash_id = 0;

	/* Read Flash ID */
	flash_id = sFLASH_ReadID();

	/* Check if the ID matches MX25L6433F */
	if (flash_id != sFLASH_MX25L6433F_ID) {
		return MX25L6433F_CHIP_ERR;
	}

	return MX25L6433F_OK;
}

KeepInCompilation uint32_t HAL_GetTick(void)
{
    return uwTick;
}

/**
 * Description :
 * Read data from the device
 * Inputs    :
 *      Address       : Write location
 *      Size          : Length in bytes
 *      buffer        : Address where to get the data to write
 * outputs   :
 *      R0             : "1" 			: Operation succeeded
 * 			  "0" 			: Operation failure
 * Note: Mandatory for all types except SRAM and PSRAM
 */
int Read(uint32_t Address, uint32_t Size, uint8_t *buffer)
{
	sFLASH_ReadBuffer(buffer, Address, Size);
	return 1;
}
void HAL_Delay(uint32_t Delay)
{
  int i=0;
  for (i=0; i<0x1000; i++);
}

KeepInCompilation HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
	uwTick += (uint32_t)uwTickFreq;
	return HAL_OK;
}

//uint32_t HAL_GetTick(void)
//{
//        return 1;
//}
