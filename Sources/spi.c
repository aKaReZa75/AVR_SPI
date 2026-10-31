/**
 ******************************************************************************
 * @file     spi.c
 * @brief    SPI master (hardware SPI) library implementation for AVR
 * 
 * @author   Hossein Bagheri
 * @github   https://github.com/aKaReZa75
 * 
 * @note     This file provides:
 *           - SPI master initialization (Mode 0, MSB first, 1MHz at 16MHz CPU)
 *           - Blocking single byte transfer using the SPIF flag
 *           - Buffer write, buffer read and sequential write-then-read
 *             functions with automatic Chip Select (CS) handling
 * 
 * @note     FUNCTION SUMMARY:
 *           - spi_Sequential : Send a buffer then receive a buffer in one CS frame
 *           - spi_Read       : Receive a buffer with CS handling
 *           - spi_Write      : Send a buffer with CS handling
 *           - spi_Transfer   : Send and receive one byte (full duplex)
 *           - spi_Init       : Configure SPI pins and peripheral in master mode
 * 
 * @note     For detailed documentation with examples, visit:
 *           https://github.com/aKaReZa75/AVR_SPI
 ******************************************************************************
 */

#include "spi.h"


/* ============================================================================
 *                         TRANSFER FUNCTIONS
 * ============================================================================ */

/* -------------------------------------------------------
 * @brief Write a buffer then read a buffer in a single CS frame
 * @param _txData Pointer to the data to send
 * @param _txLen  Number of bytes to send
 * @param _rxData Pointer to the buffer that receives the data
 * @param _rxLen  Number of bytes to read
 * @retval None
 * @note CS stays LOW during the whole sequence
 *       Bytes received during the write phase are discarded
 *       Dummy bytes are transmitted during the read phase
 * ------------------------------------------------------- */
void spi_Sequential(uint8_t *_txData, uint16_t _txLen, uint8_t *_rxData, uint16_t _rxLen)
{
    bitClear(__SPI_CS_Control, __SPI_CS_Pin);               /**< Select slave (CS LOW) */
    while(_txLen--)
    {
        (void)spi_Transfer(*_txData++);                     /**< Send byte, ignore received data */
    };
    while(_rxLen--)
    {
        *_rxData++ = spi_Transfer(__SPI_Dummy_Write);       /**< Clock in one byte */
    };    
    bitSet(__SPI_CS_Control, __SPI_CS_Pin);                 /**< Deselect slave (CS HIGH) */
};


/* -------------------------------------------------------
 * @brief Read a buffer from the slave
 * @param _Data Pointer to the buffer that receives the data
 * @param _Len  Number of bytes to read
 * @retval None
 * @note CS is pulled LOW before and HIGH after the transfer
 *       Dummy bytes are transmitted while reading
 * ------------------------------------------------------- */
void spi_Read(uint8_t* _Data, uint16_t _Len)
{
    bitClear(__SPI_CS_Control, __SPI_CS_Pin);               /**< Select slave (CS LOW) */
    while(_Len--)
    {
        *_Data++ = spi_Transfer(__SPI_Dummy_Write);         /**< Clock in one byte */
    };
    bitSet(__SPI_CS_Control, __SPI_CS_Pin);                 /**< Deselect slave (CS HIGH) */
};


/* -------------------------------------------------------
 * @brief Write a buffer to the slave
 * @param _Data Pointer to the data to send
 * @param _Len  Number of bytes to send
 * @retval None
 * @note CS is pulled LOW before and HIGH after the transfer
 *       Received bytes are discarded
 * ------------------------------------------------------- */
void spi_Write(uint8_t* _Data, uint16_t _Len)
{
    bitClear(__SPI_CS_Control, __SPI_CS_Pin);               /**< Select slave (CS LOW) */
    while(_Len--)
    {
        (void)spi_Transfer(*_Data++);                       /**< Send byte, ignore received data */
    };
    bitSet(__SPI_CS_Control, __SPI_CS_Pin);                 /**< Deselect slave (CS HIGH) */
};


/* -------------------------------------------------------
 * @brief Transfer one byte over SPI (full duplex)
 * @param _Data Byte to transmit
 * @retval Byte received from the slave
 * @note Blocking: waits until the SPIF flag is set
 *       Does not control the CS pin
 * ------------------------------------------------------- */
uint8_t spi_Transfer(uint8_t _Data)
{
  SPDR = _Data;                                             /**< Load data register, starts the transfer */
  bitWaitHigh(SPSR, SPIF);                                  /**< Wait for transfer complete flag */
  return SPDR;                                              /**< Return received byte */
};


/* ============================================================================
 *                       INITIALIZATION FUNCTION
 * ============================================================================ */

/* -------------------------------------------------------
 * @brief Initialize SPI peripheral in master mode
 * @retval None
 * @note Configuration:
 *       - SCK (PB5), MOSI (PB3), CS: outputs
 *       - MISO (PB4): input
 *       - Prescaler /16 -> 1MHz at 16MHz CPU clock
 *       - Mode 0 (CPOL=0, CPHA=0), MSB first, master mode
 *       - CS is set HIGH (slave deselected)
 * @note Must be called before any SPI operation
 * ------------------------------------------------------- */
void spi_Init(void)
{
  GPIO_Config_OUTPUT(DDRB, 5); /**< Config SCK as output */
  GPIO_Config_INPUT (DDRB, 4); /**< Config MISO as input */  
  GPIO_Config_OUTPUT(DDRB, 3); /**< Config MOSI as output */
  GPIO_Config_OUTPUT(__SPI_CS_Config, __SPI_CS_Pin);  /**< Config CS as output */ 
  bitSet(__SPI_CS_Control, __SPI_CS_Pin); 

  /* Prescaler: 16 => 16MHz/16 = 1MHz */
  bitSet  (SPCR, SPR0);
  bitClear(SPCR, SPR1);
  bitClear(SPSR, SPI2X);   

  /* SPI MODE => 0 */
  bitClear(SPCR, CPOL);
  bitClear(SPCR, CPHA);

  bitSet  (SPCR, SPE);  /* Enable the SPI */
  bitClear(SPCR, DORD); /* MSB First */
  bitSet  (SPCR, MSTR); /* Master Mode */
};
