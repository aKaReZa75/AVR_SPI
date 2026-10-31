/**
 ******************************************************************************
 * @file     spi.h
 * @brief    SPI master (hardware SPI) library header for AVR
 * 
 * @author   Hossein Bagheri
 * @github   https://github.com/aKaReZa75
 * 
 * @note     This header file provides definitions for:
 *           - Chip Select (CS) pin mapping
 *           - Dummy byte used for read operations
 *           - SPI master initialization (Mode 0, MSB first, 1MHz at 16MHz CPU)
 *           - Single byte transfer, buffer write, buffer read and sequential
 *             write-then-read functions
 * 
 * @note     FUNCTION SUMMARY:
 *           - spi_Init       : Configure SPI pins and peripheral in master mode
 *           - spi_Transfer   : Send and receive one byte (full duplex)
 *           - spi_Write      : Send a buffer with CS handling
 *           - spi_Read       : Receive a buffer with CS handling
 *           - spi_Sequential : Send a buffer then receive a buffer in one CS frame
 * 
 * @note     Hardware pins (ATmega328P):
 *           - SCK  : PB5 (output)
 *           - MISO : PB4 (input)
 *           - MOSI : PB3 (output)
 *           - CS   : PB2 by default (configurable below)
 *           If another CS pin is used, keep PB2 (SS) configured as output,
 *           otherwise a LOW level on it can switch the SPI back to slave mode.
 * 
 * @note     For detailed documentation with examples, visit:
 *           https://github.com/aKaReZa75/AVR_SPI
 ******************************************************************************
 */
#ifndef _spi_H_
#define _spi_H_

#include "aKaReZa.h"


/* ============================================================================
 *                    CRITICAL DEPENDENCY CHECK
 * ============================================================================
 *  This library requires the aKaReZa.h base library to compile correctly.
 *  If the file is missing, please download it or contact for support.
 * ============================================================================ */
#ifndef _aKaReZa_H_
    #warning "============================================================"
    #warning " [WARNING] Missing required dependency: aKaReZa.h"
    #warning "------------------------------------------------------------"
    #warning "  This module depends on the aKaReZa.h base library."
    #warning "  Please download it from: https://github.com/aKaReZa75/AVR_RawProject"
    #warning "  Or contact for support: akaReza75@gmail.com"
    #warning "------------------------------------------------------------"
    #error   "Compilation aborted: Required file 'aKaReZa.h' not found!"
    #warning "============================================================"
#endif


/* ============================================================================
 *                         PIN DEFINITIONS
 * ============================================================================ */

/* -------------------------------------------------------
 * @brief Chip Select (CS) pin mapping
 * @note Change these macros to use a different GPIO as CS
 * ------------------------------------------------------- */
#define __SPI_CS_Config  DDRB            /**< CS direction register */
#define __SPI_CS_Control PORTB           /**< CS output register */
#define __SPI_CS_Pin     2               /**< CS pin number (PB2) */


/* ============================================================================
 *                         PROTOCOL PARAMETERS
 * ============================================================================ */

/* -------------------------------------------------------
 * @brief Dummy byte sent while reading from the slave
 * @note The master must clock out a byte to receive one
 * ------------------------------------------------------- */
#define __SPI_Dummy_Write 0xFF           /**< Byte transmitted during read operations */


/* ============================================================================
 *                         FUNCTION PROTOTYPES
 * ============================================================================ */

/* -------------------------------------------------------
 * @brief Initialize SPI peripheral in master mode
 * @retval None
 * @note Configures SCK/MOSI/CS as outputs and MISO as input
 *       Mode 0 (CPOL=0, CPHA=0), MSB first, prescaler /16 (1MHz at 16MHz)
 *       CS is set HIGH (slave deselected)
 *       Must be called before any SPI operation
 * ------------------------------------------------------- */
void spi_Init(void);

/* -------------------------------------------------------
 * @brief Transfer one byte over SPI (full duplex)
 * @param _Data Byte to transmit
 * @retval Byte received from the slave
 * @note Blocking: waits until the transfer completes (SPIF flag)
 *       Does not control the CS pin
 * ------------------------------------------------------- */
uint8_t spi_Transfer(uint8_t _Data);

/* -------------------------------------------------------
 * @brief Write a buffer to the slave
 * @param _Data Pointer to the data to send
 * @param _Len  Number of bytes to send
 * @retval None
 * @note CS is pulled LOW before and HIGH after the transfer
 *       Received bytes are discarded
 * ------------------------------------------------------- */
void spi_Write(uint8_t* _Data, uint16_t _Len);

/* -------------------------------------------------------
 * @brief Read a buffer from the slave
 * @param _Data Pointer to the buffer that receives the data
 * @param _Len  Number of bytes to read
 * @retval None
 * @note CS is pulled LOW before and HIGH after the transfer
 *       Dummy bytes (__SPI_Dummy_Write) are transmitted while reading
 * ------------------------------------------------------- */
void spi_Read(uint8_t* _Data, uint16_t _Len);

/* -------------------------------------------------------
 * @brief Write a buffer then read a buffer in a single CS frame
 * @param _txData Pointer to the data to send (e.g. command/address)
 * @param _txLen  Number of bytes to send
 * @param _rxData Pointer to the buffer that receives the data
 * @param _rxLen  Number of bytes to read
 * @retval None
 * @note CS stays LOW during the whole sequence, which is required by
 *       most devices (e.g. flash memories, sensors) for command + response
 * ------------------------------------------------------- */
void spi_Sequential(uint8_t *_txData, uint16_t _txLen, uint8_t *_rxData, uint16_t _rxLen);

#endif /* _spi_H_ */
