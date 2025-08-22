#include "MB1642BDataReader.h"
#include "stm32u3xx_hal.h"
#include "main.h"

#define CMD_RDID 0x9F
#define CMD_READ 0x03
#define CMD_WREN 0x06
#define CMD_PP 0x02
#define CMD_RDSR 0x05
#define CMD_SE 0xD8
#define STATUS_WIP 0x01

extern SPI_HandleTypeDef hspi2;
// private variables
// DMA handles for reading pixels from SPI peripheral
extern DMA_HandleTypeDef handle_GPDMA1_Channel1; // rx

extern DMA_HandleTypeDef handle_GPDMA1_Channel2; // tx

////Status flag. Non-zero when receiving data
static volatile uint8_t isReceivingData = 0;
//
void readData(uint32_t address24, uint8_t *buffer, uint32_t length)
{
    FLASH_CS_GPIO_Port->BRR = FLASH_CS_Pin;

    SPI_2LINES_TX(&hspi2);
    // Set Size in CR2->TSIZE
    MODIFY_REG(hspi2.Instance->CR2, SPI_CR2_TSIZE, 1);

    // Enable SPI if disabled
    if ((hspi2.Instance->CR1 & SPI_CR1_SPE) == 0)
        hspi2.Instance->CR1 |= SPI_CR1_SPE;

    // Start Master transmit
    SET_BIT(hspi2.Instance->CR1, SPI_CR1_CSTART);

    while ((hspi2.Instance->SR & SPI_SR_TXP) == 0)
        ;
    *((__IO uint8_t *)&hspi2.Instance->TXDR) = CMD_READ;
    while ((hspi2.Instance->SR & SPI_SR_EOT) == 0)
        ;

    // Clear TXTFC and EOTC flags
    SET_BIT(hspi2.Instance->IFCR, SPI_IFCR_EOTC);
    SET_BIT(hspi2.Instance->IFCR, SPI_IFCR_TXTFC);

    __HAL_SPI_DISABLE(&hspi2);

    // Set Size in CR2->TSIZE
    MODIFY_REG(hspi2.Instance->CR2, SPI_CR2_TSIZE, 3);

    // Enable SPI if disabled
    if ((hspi2.Instance->CR1 & SPI_CR1_SPE) == 0)
        hspi2.Instance->CR1 |= SPI_CR1_SPE;

    // Start Master transmit
    SET_BIT(hspi2.Instance->CR1, SPI_CR1_CSTART);

    while ((hspi2.Instance->SR & SPI_SR_TXP) == 0)
        ;
    // clock out address
    *((__IO uint8_t *)&hspi2.Instance->TXDR) = (address24 >> 16) & 0xFF;

    while ((hspi2.Instance->SR & SPI_SR_TXP) == 0)
        ;
    *((__IO uint8_t *)&hspi2.Instance->TXDR) = (address24 >> 8) & 0xFF;

    while ((hspi2.Instance->SR & SPI_SR_TXP) == 0)
        ;
    *((__IO uint8_t *)&hspi2.Instance->TXDR) = address24 & 0xFF;

    while ((hspi2.Instance->SR & SPI_SR_EOT) == 0)
        ;
    SET_BIT(hspi2.Instance->IFCR, SPI_IFCR_EOTC);
    SET_BIT(hspi2.Instance->IFCR, SPI_IFCR_TXTFC);

    __HAL_SPI_DISABLE(&hspi2);

    SPI_2LINES_RX(&hspi2);

    MODIFY_REG(hspi2.Instance->CR2, SPI_CR2_TSIZE, length);
    __HAL_SPI_ENABLE(&hspi2);
    /* Master transfer start */
    SET_BIT(hspi2.Instance->CR1, SPI_CR1_CSTART);

    while (length--)
    {
        while ((hspi2.Instance->SR & SPI_SR_RXP) == 0)
            ;
        *((uint8_t *)buffer) = *((__IO uint8_t *)&hspi2.Instance->RXDR);
        buffer++;
    }
    /* Wait until the bus is ready before releasing Chip select */
    while ((hspi2.Instance->SR & SPI_SR_EOT) == 0)
        ;
    __HAL_SPI_CLEAR_EOTFLAG(&hspi2);
    __HAL_SPI_CLEAR_TXTFFLAG(&hspi2);
    __HAL_SPI_DISABLE(&hspi2);
    FLASH_CS_GPIO_Port->BSRR = FLASH_CS_Pin;
}

void readDataDMA(uint32_t address24, uint8_t *buffer, uint32_t length)
{
    //     Pull Flash CS pin low
    isReceivingData = 1;
    FLASH_CS_GPIO_Port->BRR = FLASH_CS_Pin;

    SPI_2LINES_TX(&hspi2);

    // Set Size in CR2->TSIZE
    MODIFY_REG(hspi2.Instance->CR2, SPI_CR2_TSIZE, 1);

    // Enable SPI if disabled
    if ((hspi2.Instance->CR1 & SPI_CR1_SPE) == 0)
        hspi2.Instance->CR1 |= SPI_CR1_SPE;

    // Start Master transmit
    SET_BIT(hspi2.Instance->CR1, SPI_CR1_CSTART);

    while ((hspi2.Instance->SR & SPI_SR_TXP) == 0)
        ;
    *((__IO uint8_t *)&hspi2.Instance->TXDR) = CMD_READ;
    while ((hspi2.Instance->SR & SPI_SR_EOT) == 0)
        ;

    // Clear TXTFC and EOTC flags
    SET_BIT(hspi2.Instance->IFCR, SPI_IFCR_EOTC);
    SET_BIT(hspi2.Instance->IFCR, SPI_IFCR_TXTFC);

    __HAL_SPI_DISABLE(&hspi2);

    // Set Size in CR2->TSIZE
    MODIFY_REG(hspi2.Instance->CR2, SPI_CR2_TSIZE, 3);

    // Enable SPI if disabled
    if ((hspi2.Instance->CR1 & SPI_CR1_SPE) == 0)
        hspi2.Instance->CR1 |= SPI_CR1_SPE;

    // Start Master transmit
    SET_BIT(hspi2.Instance->CR1, SPI_CR1_CSTART);

    while ((hspi2.Instance->SR & SPI_SR_TXP) == 0)
        ;
    // clock out address
    *((__IO uint8_t *)&hspi2.Instance->TXDR) = (address24 >> 16) & 0xFF;

    while ((hspi2.Instance->SR & SPI_SR_TXP) == 0)
        ;
    *((__IO uint8_t *)&hspi2.Instance->TXDR) = (address24 >> 8) & 0xFF;

    while ((hspi2.Instance->SR & SPI_SR_TXP) == 0)
        ;
    *((__IO uint8_t *)&hspi2.Instance->TXDR) = address24 & 0xFF;

    while ((hspi2.Instance->SR & SPI_SR_EOT) == 0)
        ;
    SET_BIT(hspi2.Instance->IFCR, SPI_IFCR_EOTC);
    SET_BIT(hspi2.Instance->IFCR, SPI_IFCR_TXTFC);

    __HAL_SPI_DISABLE(&hspi2);

    SPI_2LINES_RX(&hspi2);

    /* Clear RXDMAEN bit */
    CLEAR_BIT(hspi2.Instance->CFG1, SPI_CFG1_RXDMAEN);

    /* Configure the DMA channel data size */
    MODIFY_REG(handle_GPDMA1_Channel1.Instance->CBR1, DMA_CBR1_BNDT,
               (length & DMA_CBR1_BNDT));

    /* Clear all interrupt flags */
    __HAL_DMA_CLEAR_FLAG(&handle_GPDMA1_Channel1,
                         DMA_FLAG_TC | DMA_FLAG_HT | DMA_FLAG_DTE | DMA_FLAG_ULE | DMA_FLAG_USE | DMA_FLAG_SUSP | DMA_FLAG_TO);

    /* Configure DMA channel source address */
    handle_GPDMA1_Channel1.Instance->CSAR = (uint32_t)&hspi2.Instance->RXDR;
    handle_GPDMA1_Channel1.Instance->CDAR = (uint32_t)buffer;
    /* Enable common interrupts: Transfer Complete and Transfer Errors ITs */
    __HAL_DMA_ENABLE_IT(&handle_GPDMA1_Channel1,
                        (DMA_IT_TC | DMA_IT_DTE | DMA_IT_ULE | DMA_IT_USE | DMA_IT_TO));
    /* Enable DMA channel */
    __HAL_DMA_ENABLE(&handle_GPDMA1_Channel1);
    MODIFY_REG(hspi2.Instance->CR2, SPI_CR2_TSIZE, length);
    /* Enable Rx DMA Request */
    SET_BIT(hspi2.Instance->CFG1, SPI_CFG1_RXDMAEN);

    /* Enable SPI peripheral */
    __HAL_SPI_ENABLE(&hspi2);

    /* Master transfer start */
    SET_BIT(hspi2.Instance->CR1, SPI_CR1_CSTART);
}
void DataReader_DMACallback()
{
    /* Transfer Complete Interrupt management ***************************************************************************/
    if (__HAL_DMA_GET_FLAG(&handle_GPDMA1_Channel1, DMA_FLAG_TC) != 0U)
    {
        /* Check if interrupt source is enabled */
        if (__HAL_DMA_GET_IT_SOURCE(&handle_GPDMA1_Channel1, DMA_IT_TC) != 0U)
        {
            /* Clear TC and HT transfer flags */
            __HAL_DMA_CLEAR_FLAG(&handle_GPDMA1_Channel1,
                                 (DMA_FLAG_TC | DMA_FLAG_HT));
            __HAL_DMA_DISABLE(&handle_GPDMA1_Channel1);
            __HAL_SPI_DISABLE(&hspi2);
        }
    }

    FLASH_CS_GPIO_Port->BSRR = FLASH_CS_Pin;

    isReceivingData = 0;
}

void DataReader_ReadData(uint32_t address24, uint8_t *buffer, uint32_t length)
{
    readData(address24, buffer, length);
}

void DataReader_StartDMAReadData(uint32_t address24, uint8_t *buffer,
                                 uint32_t length)
{
    readDataDMA(address24, buffer, length);
}

uint32_t DataReader_IsReceivingData(void)
{
    return isReceivingData;
}

void DataReader_WaitForReceiveDone(void)
{
    while (isReceivingData)
        ;
}

void DataReader_Init(void)
{
    __HAL_SPI_ENABLE(&hspi2);
}
