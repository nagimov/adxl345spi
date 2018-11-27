# ADXL345 accelerometer reader (SPI interface)

This tiny utility provides an easy to use and reliable **non-realtime** access to
ADXL345 three-axis digital accelerometer ([datasheet][adxl_manual]) through SPI interface at a maximal sampling rate of 3200 Hz.
The output of the reader can be redirected to a standard console output or a text file.

## Usage

**Executable binary: [adxl345spi][adxl345spi_binary]**

```
Usage: adxl345spi [OPTION]...

Mandatory arguments to long options are mandatory for short options too.
  -s, --save FILE     save data to specified FILE (data printed to command-line
                      output, if not specified)
  -t, --time TIME     set the duration of data stream to TIME seconds
                      (default: 5 seconds) [integer]
  -f, --freq FREQ     set the sampling rate of data stream to FREQ samples per
                      second, 1 <= FREQ <= 3200 (default: 5 Hz) [integer]

Data is streamed in comma separated format, e. g.:
  time,     x,     y,     z
   0.0,  10.0,   0.0, -10.0
   1.0,   5.0,  -5.0,  10.0
   ...,   ...,   ...,   ...
  time shows seconds elapsed since the first reading;
  x, y and z show acceleration along x, y and z axis in fractions of <g>.

Exit status:
  0  if OK
  1  if error occurred during data reading or wrong cmdline arguments.
```

## Wiring

Fritzing diagrams for two popular revisions:

![adxl345spi_bb][adxl345spi_bb_png]

![adxl345spi_schem][adxl345spi_schem_png]

```
CS  : SPI_CE0_N (pin 24)
SDO : SPI_MISO (pin 21)
SDA : SPI_MOSI (pin 19)
SCL : SPI_CLK (pin 23)
```

Some of the ADXL345 breadboards have `VCC` pin marked as `3V3`. 

## Build

### Dependencies

This utility is built using `pigpio` library, which provides a `C` interface to the General Purpose Input Outputs
(GPIO) of Raspberry Pi. More information on provided `C` interface can be found [here][pigpio_info_C].

In order to compile `adxl345spi.c` file, install `pigpio` library first. The procedure can be shortly described as follows:

```
wget abyz.co.uk/rpi/pigpio/pigpio.zip
unzip pigpio.zip
cd PIGPIO
make -j4
sudo make install
```

More information can be found on the [download page][pigpio_download].

### Compilation

`gcc -Wall -pthread -o adxl345spi adxl345spi.c -lpigpio -lrt`

**Build is only tested on Raspbian OS.**

## Code features

Due to limitations of I2C bus, it is impossible to obtain a maximal sampling rate through I2C interface. Therefore, SPI
interface is used to get 3200 samples per second, which is the upper limit of ADXL345 chip itself. SPI bus data transfer
rate is set to 2 Mbps, to ensure no readings are lost during transmission.

For the console output, the downsampling is achieved using `sleep()` calls between transmissions, providing someway
non-stable sampling rate. Hence, console output should be only used for a demo/test cases.

File output is performed through an accurate downsampling of the entire dataset read at a highest possible sampling
rate (~30,000 Hz for 2 Mbps transfer rate). Such post-processing as FFT should be only done using a file output option,
which provides a stable sampling rate and reliable time steps.

[adxl345spi_binary]: https://github.com/nagimov/adxl345spi/raw/master/adxl345spi
[adxl_manual]: http://www.analog.com/en/products/mems/accelerometers/adxl345.html
[pigpio_info_C]: http://abyz.co.uk/rpi/pigpio/cif.html
[pigpio_download]: http://abyz.co.uk/rpi/pigpio/download.html
[adxl345spi_bb_png]: adxl345spi_bb.png
[adxl345spi_schem_png]: adxl345spi_schem.png

