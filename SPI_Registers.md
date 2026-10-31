# SPI Registers (ATmega328P)

The SPI peripheral is controlled by three registers: `SPCR` (control), `SPSR` (status), and `SPDR` (data). This page describes every bit and shows how `spi_Init()` and `spi_Transfer()` use them.

---

## SPCR - SPI Control Register

| Bit | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
| --- | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: |
| **Name** | SPIE | SPE | DORD | MSTR | CPOL | CPHA | SPR1 | SPR0 |

| Bit | Name | Description | Library value |
| :-: | ---- | ----------- | :-----------: |
| 7 | **SPIE** | SPI interrupt enable. When set (with global interrupts enabled), an interrupt occurs when SPIF is set | `0` (polling) |
| 6 | **SPE** | SPI enable. The SPI operates only when this bit is set | `1` |
| 5 | **DORD** | Data order: `0` = MSB first, `1` = LSB first | `0` (MSB first) |
| 4 | **MSTR** | Master/Slave select: `1` = master, `0` = slave | `1` (master) |
| 3 | **CPOL** | Clock polarity: SCK idle level (`0` = LOW, `1` = HIGH) | `0` |
| 2 | **CPHA** | Clock phase: sampling edge (`0` = leading, `1` = trailing) | `0` |
| 1 | **SPR1** | Clock rate select bit 1 | `0` |
| 0 | **SPR0** | Clock rate select bit 0 | `1` |

See [SPI Modes and Bit Order](./SPI_Modes.md) for CPOL, CPHA, and DORD, and [SPI Clock Rate](./SPI_ClockRate.md) for SPR1, SPR0.

---

## SPSR - SPI Status Register

| Bit | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
| --- | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: |
| **Name** | SPIF | WCOL | - | - | - | - | - | SPI2X |

| Bit | Name | Description |
| :-: | ---- | ----------- |
| 7 | **SPIF** | SPI interrupt flag. Set by hardware when a serial transfer is complete. Cleared by reading `SPSR` (with SPIF set) and then accessing `SPDR` |
| 6 | **WCOL** | Write collision flag. Set if `SPDR` is written while a transfer is in progress. Cleared by reading `SPSR` (with WCOL set) and then accessing `SPDR` |
| 5:1 | - | Reserved |
| 0 | **SPI2X** | Double SPI speed bit. When set, the clock divider is halved (see [SPI Clock Rate](./SPI_ClockRate.md)) |

The library waits for the end of a byte with:

```c
bitWaitHigh(SPSR, SPIF);   /**< Wait for transfer complete flag */
```

---

## SPDR - SPI Data Register

| Bit | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
| --- | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: |
| **Name** | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 |

- **Writing** to `SPDR` loads the transmit shift register and **starts the transfer**.
- **Reading** from `SPDR` returns the byte that was received.

```c
SPDR = _Data;               /**< Load data register, starts the transfer */
bitWaitHigh(SPSR, SPIF);    /**< Wait for the end of the transfer */
return SPDR;                /**< Read the received byte (also clears SPIF) */
```

> [!NOTE]
> SPI is full duplex: every transmitted bit is paired with a received bit. After a transfer, `SPDR` always holds the received byte, even if you only wanted to send.

---

## Pin Configuration

When the SPI is enabled, some pin directions are controlled by hardware and some must be set by the user. In master mode:

| Pin | Signal | Direction | Set by |
| --- | ------ | --------- | ------ |
| PB5 | SCK  | Output | User (`DDRB`) |
| PB4 | MISO | Input  | Hardware (always input in master mode) |
| PB3 | MOSI | Output | User (`DDRB`) |
| PB2 | SS   | See below | User (`DDRB`) |

> [!IMPORTANT]
> **The SS pin (PB2) in master mode:**
> - If PB2 is an **output**, it is a normal GPIO and does not affect the SPI. The library uses it as CS by default.
> - If PB2 is an **input**, a **LOW** level on it makes the hardware clear `MSTR`, and the SPI switches to slave mode. After that, `SPIF` never sets in master transfers and `spi_Transfer()` hangs.
>
> If you use a different CS pin, keep PB2 configured as an output.

---

## How `spi_Init()` Uses the Registers

```c
/* Pins */
GPIO_Config_OUTPUT(DDRB, 5);  /* SCK  as output */
GPIO_Config_INPUT (DDRB, 4);  /* MISO as input  */
GPIO_Config_OUTPUT(DDRB, 3);  /* MOSI as output */
/* CS pin as output, set HIGH (slave deselected) */

/* Clock: prescaler /16 => 1 MHz at 16 MHz */
bitSet  (SPCR, SPR0);
bitClear(SPCR, SPR1);
bitClear(SPSR, SPI2X);

/* Mode 0 */
bitClear(SPCR, CPOL);
bitClear(SPCR, CPHA);

bitSet  (SPCR, SPE);   /* Enable the SPI */
bitClear(SPCR, DORD);  /* MSB first */
bitSet  (SPCR, MSTR);  /* Master mode */
```

The resulting `SPCR` value is `0b01010001` (`0x51`): SPE, MSTR, and SPR0 set.

---

## Power Reduction

The SPI module can be switched off in the Power Reduction Register (`PRR`) with the **PRSPI** bit (bit 2) to save power. If PRSPI is set, the SPI cannot be used. After reset it is cleared, so no action is needed unless your code uses power reduction.

---

## Related

- [SPI Clock Rate](./SPI_ClockRate.md)
- [SPI Modes and Bit Order](./SPI_Modes.md)
- [API Reference](./API_Reference.md)
