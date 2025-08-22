
#include "main.h"
#include "spi.h"
#include "gpio.h"
#include "mx25l6433f.h"
#include "MB1642BDataReader.h" // readData fonksiyonu için

#define LOADER_START_ADDR 0x90000000
#define FLASH_SIZE 0x800000 // 8MB flash size (64Mbit)
#define LOADER_OK 0x1
#define LOADER_FAIL 0x0
extern void SystemClock_Config(void);

extern char __bss_start__;
extern char __bss_end__;



/**
 * @brief  System initialization.
 * @param  None
 * @retval  1      : Operation succeeded
 * @retval  0      : Operation failed
 */
int Init(void)
{
  int32_t result=0;

  __disable_irq();

  /* Init structs to Zero */
  char *startadd = &__bss_start__;
  uint32_t size  = &__bss_end__ - &__bss_start__;
  memset(startadd, 0, size);

  /* init system */
  SystemInit();
  HAL_Init();

  /* Configure the system clock  */
  result = SystemClockInit();
  if(result != 1){
    return 0;
  }

  /* Initialize peripherals */
  MX_ICACHE_Init();
  MX_USART1_UART_Init();

  /* QuadSPI De-Init */
  MX_OCTOSPI1_DeInit();

  /* QuadSPI Init */
  MX_OCTOSPI1_Init();

  if(PY25Q64_Init() != PY25Q64_OK){
      return 0;
  }

  PY25Q64_AutoPollingMemReady();

  /* Set Memory Mapped Mode */
  result = PY25Q64_MemoryMappedMode();

  if(result != PY25Q64_OK){
    return 0;
  }

  __enable_irq();

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
KeepInCompilation int Write (uint32_t Address, uint32_t Size, uint8_t* buffer)
{
  __disable_irq();

  /* QuadSPI De-Init */
  MX_OCTOSPI1_DeInit();

  /* QuadSPI Init */
  MX_OCTOSPI1_Init();

  Address = Address & 0x0FFFFFFF;

  /* Writes an amount of data to the QSPI memory */
  if(PY25Q64_Program(buffer, Size, Address) != PY25Q64_OK){
    return 0;
  }

  __enable_irq();

  return 1;
}

/**
 * @brief 	 Full erase of the device
 * @param 	 Parallelism : 0
 * @retval  1           : Operation succeeded
 * @retval  0           : Operation failed
 */
KeepInCompilation int MassErase (uint32_t Parallelism )
{
  __disable_irq();

  /* QuadSPI De-Init */
  MX_OCTOSPI1_DeInit();

  /* QuadSPI Init */
  MX_OCTOSPI1_Init();

  PY25Q64_AutoPollingMemReady();

  /* Erase the entire QSPI memory */
  if (PY25Q64_MassErase() != PY25Q64_OK){
      return 0;
  }

  __enable_irq();

   return 1;
}

/**
 * @brief   Sector erase.
 * @param   EraseStartAddress :  erase start address
 * @param   EraseEndAddress   :  erase end address
 * @retval  None
 */
KeepInCompilation int SectorErase (uint32_t EraseStartAddress ,uint32_t EraseEndAddress)
{
  __disable_irq();

  uint32_t BlockAddr = 0;
  EraseStartAddress &= 0x0FFFFFFF;
  EraseEndAddress &= 0x0FFFFFFF;
  EraseStartAddress = EraseStartAddress - EraseStartAddress % 0x10000;

  /* QuadSPI De-Init */
  MX_OCTOSPI1_DeInit();

  /* QuadSPI Init */
  MX_OCTOSPI1_Init();

  while(EraseEndAddress >= EraseStartAddress){
    BlockAddr = EraseStartAddress / (MEM_BLOCK_SIZE * 1024U) ;

    /* Erases the specified block of the QSPI memory */
    PY25Q64_BlockErase(BlockAddr);

    /* Reads current status of the QSPI memory */
    EraseStartAddress += 0x10000;
  }

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
uint32_t CheckSum(uint32_t StartAddress, uint32_t Size, uint32_t InitVal)
{
  __disable_irq();

  uint8_t missalignementAddress = StartAddress%4;
  uint8_t missalignementSize = Size ;
  int cnt;
  uint32_t Val;

  StartAddress-=StartAddress%4;
  Size += (Size%4==0)?0:4-(Size%4);

  for(cnt=0; cnt<Size ; cnt+=4)
  {
    Val = *(uint32_t*)StartAddress;
    if(missalignementAddress)
    {
      switch (missalignementAddress)
      {
      case 1:
        InitVal += (uint8_t) (Val>>8 & 0xff);
        InitVal += (uint8_t) (Val>>16 & 0xff);
        InitVal += (uint8_t) (Val>>24 & 0xff);
        missalignementAddress-=1;
        break;
      case 2:
        InitVal += (uint8_t) (Val>>16 & 0xff);
        InitVal += (uint8_t) (Val>>24 & 0xff);
        missalignementAddress-=2;
        break;
      case 3:
        InitVal += (uint8_t) (Val>>24 & 0xff);
        missalignementAddress-=3;
        break;
      }
    }
    else if((Size-missalignementSize)%4 && (Size-cnt) <=4)
    {
      switch (Size-missalignementSize)
      {
      case 1:
        InitVal += (uint8_t) Val;
        InitVal += (uint8_t) (Val>>8 & 0xff);
        InitVal += (uint8_t) (Val>>16 & 0xff);
        missalignementSize-=1;
        break;
      case 2:
        InitVal += (uint8_t) Val;
        InitVal += (uint8_t) (Val>>8 & 0xff);
        missalignementSize-=2;
        break;
      case 3:
        InitVal += (uint8_t) Val;
        missalignementSize-=3;
        break;
      }
    }
    else
    {
      InitVal += (uint8_t) Val;
      InitVal += (uint8_t) (Val>>8 & 0xff);
      InitVal += (uint8_t) (Val>>16 & 0xff);
      InitVal += (uint8_t) (Val>>24 & 0xff);
    }
    StartAddress+=4;
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
KeepInCompilation uint64_t Verify (uint32_t MemoryAddr, uint32_t RAMBufferAddr, uint32_t Size, uint32_t missalignement)
{
  __disable_irq();

  uint32_t VerifiedData = 0, InitVal = 0;
  uint64_t checksum = 0;
  Size*=4;

  checksum = CheckSum((uint32_t)MemoryAddr + (missalignement & 0xf), Size - ((missalignement >> 16) & 0xF), InitVal);
  while (Size>VerifiedData)
  {
    if ( *(uint8_t*)MemoryAddr++ != *((uint8_t*)RAMBufferAddr + VerifiedData))
      return ((checksum<<32) + (MemoryAddr + VerifiedData));

    VerifiedData++;
  }

  __enable_irq();

  return (checksum<<32);
}

/**
 * @brief SPI2 Deinitialization Function
 * @param None
 * @retval None
 */
void MX_SPI2_DeInit(void)
{
  /* USER CODE BEGIN SPI2_DeInit 0 */

  /* USER CODE END SPI2_DeInit 0 */

  /* Disable SPI2 peripheral */
  if (HAL_SPI_DeInit(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN SPI2_DeInit 1 */

  /* USER CODE END SPI2_DeInit 1 */
}
