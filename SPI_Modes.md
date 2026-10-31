# SPI Modes and Bit Order

An SPI slave expects data to be shifted with a specific clock polarity, a specific sampling edge, and a specific bit order. These are set by three bits in the `SPCR` register: **CPOL**, **CPHA**, and **DORD**.

## Clock Polarity and Phase

- **CPOL** (Clock Polarity): the level of SCK while the bus is idle
  - `0` : SCK is LOW when idle
  - `1` : SCK is HIGH when idle
- **CPHA** (Clock Phase): which clock edge samples the data
  - `0` : data is sampled on the **leading** (first) edge of SCK, and changed on the trailing edge
  - `1` : data is sampled on the **trailing** (second) edge of SCK, and changed on the leading edge

## The Four SPI Modes

| Mode | CPOL | CPHA | SCK idle level | Leading edge | Trailing edge | Data sampled on | Data changed on |
| :--: | :--: | :--: | :------------: | :----------: | :-----------: | :-------------: | :-------------: |
| **0** | 0 | 0 | LOW  | Rising  | Falling | Rising edge  | Falling edge |
| **1** | 0 | 1 | LOW  | Rising  | Falling | Falling edge | Rising edge  |
| **2** | 1 | 0 | HIGH | Falling | Rising  | Falling edge | Rising edge  |
| **3** | 1 | 1 | HIGH | Falling | Rising  | Rising edge  | Falling edge |

> [!TIP]
> Modes 0 and 3 both sample data on the **rising edge**; they differ only in the idle level of SCK. Many devices accept either of the two, so check the datasheet.

## Library Default

`spi_Init()` selects **Mode 0** and **MSB first**:

```c
/* SPI MODE => 0 */
bitClear(SPCR, CPOL);
bitClear(SPCR, CPHA);

bitClear(SPCR, DORD); /* MSB First */
```

## Changing the Mode

Edit the `CPOL` and `CPHA` lines in `spi_Init()`:

```c
/* Mode 1 */
bitClear(SPCR, CPOL);
bitSet  (SPCR, CPHA);

/* Mode 2 */
bitSet  (SPCR, CPOL);
bitClear(SPCR, CPHA);

/* Mode 3 */
bitSet  (SPCR, CPOL);
bitSet  (SPCR, CPHA);
```

## Bit Order (DORD)

| DORD | Bit order | Description |
| :--: | --------- | ----------- |
| `0` | **MSB first** | The most significant bit is sent first (default; used by most devices) |
| `1` | **LSB first** | The least significant bit is sent first |

```c
bitClear(SPCR, DORD); /* MSB first (default) */
bitSet  (SPCR, DORD); /* LSB first */
```

## Finding the Right Mode for a Device

1. Open the slave's datasheet and look for the **SPI timing diagram**, or for the words *CPOL*, *CPHA*, or *SPI Mode*.
2. Check whether SCK is LOW or HIGH when idle (this gives CPOL).
3. Check on which edge the slave samples MOSI (this gives CPHA together with CPOL).
4. Check the bit order (MSB or LSB first).

> [!IMPORTANT]
> A wrong mode usually does not stop the communication; it produces shifted, inverted, or random-looking data. If you read garbage from a device that is correctly wired, the mode is the first thing to check.

## Related

- [SPI Clock Rate](./SPI_ClockRate.md)
- [SPI Registers](./SPI_Registers.md)
- [API Reference](./API_Reference.md)
