# SPI Library - API Reference

This section provides detailed descriptions of the functions used to interact with the SPI peripheral of the ATmega328 microcontroller. These functions are part of the custom library for handling SPI master communication, enabling single byte transfers, buffer write/read operations, and sequential write-then-read access with automatic Chip Select (CS) handling. Each function is explained with its purpose and usage, accompanied by code examples to demonstrate how they can be implemented in your projects.

> [!NOTE]
> The library and all of its APIs provided below have been developed by myself.
> This library utilizes various macros defined in the `aKaReZa.h` header file, which are designed to simplify bitwise operations and register manipulations.
> Detailed descriptions of these macros can be found at the following link:
> [https://github.com/aKaReZa75/AVR/blob/main/Macros.md](https://github.com/aKaReZa75/AVR/blob/main/Macros.md)

| Setting             | Value                          |
| ------------------- | ------------------------------ |
| **CPU Frequency**   | 16 MHz                         |
| **SCK Frequency**   | 1 MHz (prescaler /16)          |
| **Mode**            | Master                         |
| **SPI Mode**        | Mode 0 (CPOL = 0, CPHA = 0)    |
| **Bit Order**       | MSB first                      |
| **Chip Select**     | Software controlled GPIO (PB2 by default) |
| **Operation**       | Polling (blocking)             |

> [!TIP]
> To change the clock speed, SPI mode, or bit order, edit `spi_Init()`. The register settings are explained in [SPI Clock Rate](./SPI_ClockRate.md), [SPI Modes and Bit Order](./SPI_Modes.md), and [SPI Registers](./SPI_Registers.md).

---

### **Hardware Pins (ATmega328P)**

| Signal | Pin | Direction | Notes |
| ------ | --- | --------- | ----- |
| **SCK**  | PB5 | Output | Serial clock |
| **MISO** | PB4 | Input  | Data from the slave |
| **MOSI** | PB3 | Output | Data to the slave |
| **CS**   | PB2 (default) | Output | Configurable with the macros below |

> [!IMPORTANT]
> If you use a different pin as CS, keep **PB2 (SS)** configured as an **output**. If PB2 is an input and a LOW level appears on it, the hardware switches the SPI from master to slave mode, and communication stops.

---

### **Configuration Macros**

These macros are defined in `spi.h`.

| Macro | Default | Description |
| ----- | ------- | ----------- |
| `__SPI_CS_Config`  | `DDRB`  | CS direction register |
| `__SPI_CS_Control` | `PORTB` | CS output register |
| `__SPI_CS_Pin`     | `2`     | CS pin number (PB2) |
| `__SPI_Dummy_Write` | `0xFF` | Byte transmitted while reading from the slave |

**Example: using PD7 as CS**

```c
#define __SPI_CS_Config  DDRD
#define __SPI_CS_Control PORTD
#define __SPI_CS_Pin     7
```

---

### **Initialization**

```c
void spi_Init(void);
```

* Configures SCK (PB5), MOSI (PB3), and CS as outputs, and MISO (PB4) as input.
* Sets the prescaler to /16 (1 MHz at 16 MHz CPU clock).
* Selects Mode 0 (CPOL = 0, CPHA = 0), MSB first, master mode, and enables the SPI.
* Sets CS HIGH (slave deselected).
* Must be called before any SPI operation.

**Example:**

```c
#include "aKaReZa.h"
#include "spi.h"

int main(void) 
{
    spi_Init(); /**< Initialize SPI in master mode with default settings */
    while(1)
    {
        /* Your code here */
    }
}
```

---

### **Transfer One Byte**

```c
uint8_t spi_Transfer(uint8_t _Data);
```

* Sends one byte and returns the byte received at the same time (SPI is full duplex).
* Blocking: waits until the SPIF flag is set.
* **Does not control the CS pin.** You are responsible for CS when you use this function directly.

**Example:**

```c
bitClear(__SPI_CS_Control, __SPI_CS_Pin); /**< Select the slave (CS LOW) */
spi_Transfer(0x05);                       /**< Send a command byte */
uint8_t status = spi_Transfer(0xFF);      /**< Clock in the reply */
bitSet(__SPI_CS_Control, __SPI_CS_Pin);   /**< Deselect the slave (CS HIGH) */
```

---

### **Write a Buffer**

```c
void spi_Write(uint8_t* _Data, uint16_t _Len);
```

* Sends `_Len` bytes from `_Data` to the slave.
* CS is pulled LOW before the transfer and HIGH after it.
* Received bytes are discarded.

**Example:**

```c
uint8_t txBuffer[3] = {0x02, 0x00, 0x10};
spi_Write(txBuffer, 3);
```

---

### **Read a Buffer**

```c
void spi_Read(uint8_t* _Data, uint16_t _Len);
```

* Reads `_Len` bytes from the slave into `_Data`.
* Dummy bytes (`__SPI_Dummy_Write`) are transmitted while reading, because the master must generate the clock for every byte it receives.
* CS is pulled LOW before the transfer and HIGH after it.

**Example:**

```c
uint8_t rxBuffer[4];
spi_Read(rxBuffer, 4);
```

---

### **Sequential Transfer (Write-Then-Read)**

```c
void spi_Sequential(uint8_t *_txData, uint16_t _txLen, uint8_t *_rxData, uint16_t _rxLen);
```

* Sends `_txLen` bytes (usually a command and/or register address), then reads `_rxLen` bytes, all in **one CS frame**.
* CS stays LOW during the whole sequence. This is required by most devices (flash memories, sensors) to receive a command and return a response.
* Bytes received during the write phase are discarded.

**Example: reading the JEDEC ID of an SPI flash memory**

```c
uint8_t command = 0x9F;  /**< JEDEC ID command (device specific, check your datasheet) */
uint8_t id[3];
spi_Sequential(&command, 1, id, 3);
```

> [!IMPORTANT]
> Never use `spi_Write()` followed by `spi_Read()` for a command + response sequence. Each call toggles CS, so the slave sees two separate frames and the command is lost. Use `spi_Sequential()` instead.

---

### **Summary**

| API Function     | Description                                                   |
| ---------------- | ------------------------------------------------------------- |
| `spi_Init`       | Initializes the SPI peripheral in master mode.                |
| `spi_Transfer`   | Sends and receives one byte (full duplex), without CS control. |
| `spi_Write`      | Writes a buffer to the slave with automatic CS handling.      |
| `spi_Read`       | Reads a buffer from the slave with automatic CS handling.     |
| `spi_Sequential` | Writes a buffer, then reads a buffer, in a single CS frame.   |

---

## **Complete Example**

```c
#include "aKaReZa.h"
#include "spi.h"

uint8_t command = 0x9F;   /**< JEDEC ID command */
uint8_t id[3];
char debugBuffer[32];

int main(void) 
{
    usart_Init(Initialize);
    spi_Init();

    /* Send the command and read 3 ID bytes in one CS frame */
    spi_Sequential(&command, 1, id, 3);

    sprintf(debugBuffer, "ID: %02X %02X %02X", id[0], id[1], id[2]);
    usart_Putsln(debugBuffer);

    while(1)
    {
        /* Main loop */
    }
}
```

---

## **Important Notes**

### Hardware
- Connect **SCK, MISO, MOSI** to the matching pins of the slave and share a common **GND**.
- Make sure the slave and the AVR use the same logic level (for example 3.3 V devices with a 5 V AVR need level shifting).
- Check the **SPI mode** (CPOL/CPHA), the **bit order**, and the **maximum clock speed** in the slave's datasheet.

### Library Behavior
- The library is **master only** and uses **polling**. `spi_Transfer()` blocks until the byte is sent.
- There is no timeout on the SPIF wait. If the SPI leaves master mode (see the PB2 note above), `spi_Transfer()` never returns.
- The library has **one CS pin**. For several slaves, see the FAQ.
- The buffer functions take non-const pointers. To send data stored in a `const` array, copy it to a RAM buffer or cast the pointer.

---

## **Troubleshooting Guide**

**Symptom:** The program hangs inside `spi_Transfer()`
- PB2 (SS) is configured as input and is being pulled LOW, so the SPI switched to slave mode. Configure PB2 as output, or use it as CS.

**Symptom:** Slave never answers (all bytes read as `0xFF` or `0x00`)
- Check the MISO wiring and the common GND
- Verify the slave's power supply
- Verify the CS pin macros match your wiring
- Reduce the SPI clock speed

**Symptom:** Data is shifted, inverted, or garbage
- SPI mode mismatch: compare CPOL/CPHA with the slave's datasheet
- Bit order mismatch: some devices need LSB first (`DORD`)
- Clock too fast for the wiring or for the slave

**Symptom:** The command works with `spi_Transfer()` but not with `spi_Write()` + `spi_Read()`
- Use `spi_Sequential()`; the slave needs CS to stay LOW between the command and the response

---

## **FAQ (Frequently Asked Questions)**

**Q: Can I use another pin as CS?**  
A: Yes. Change the three `__SPI_CS_*` macros in `spi.h`. Keep PB2 configured as output.

**Q: How do I talk to several slaves?**  
A: Define each slave's CS pin yourself and use `spi_Transfer()` with manual CS control:
```c
bitClear(PORTD, 7);          /**< Select slave 2 */
spi_Transfer(0xAB);
bitSet(PORTD, 7);            /**< Deselect slave 2 */
```
Remember to configure each extra CS pin as an output and set it HIGH at startup.

**Q: Can the AVR act as an SPI slave?**  
A: Not with this library. It supports master mode only.

**Q: How do I send 16-bit values?**  
A: Send two bytes, most significant byte first (the library uses MSB first bit order, and most devices expect the high byte first):
```c
uint16_t value = 0x1234;
uint8_t txBuffer[2] = {(uint8_t)(value >> 8), (uint8_t)value};
spi_Write(txBuffer, 2);
```

**Q: Why is the dummy byte `0xFF`?**  
A: The master must send something to generate the clock while reading. `0xFF` keeps MOSI HIGH, which some devices (such as SD cards) require. You can change `__SPI_Dummy_Write` if your device needs a different value.

---

# 🌟 Support Me
If you found this repository useful:
- Subscribe to my [YouTube Channel](https://www.youtube.com/@aKaReZa75).
- Share this repository with others.
- Give this repository and my other repositories a star.
- Follow my [GitHub account](https://github.com/aKaReZa75).

# ✉️ Contact Me
Feel free to reach out to me through any of the following platforms:
- 📧 [Email: aKaReZa75@gmail.com](mailto:aKaReZa75@gmail.com)
- 🎥 [YouTube: @aKaReZa75](https://www.youtube.com/@aKaReZa75)
- 💼 [LinkedIn: @akareza75](https://www.linkedin.com/in/akareza75)
