# ADXL345 accelerometer reader for Raspberry Pi

This simple command line tool provides an easy to use and reliable **non-realtime** access to ADXL345 three-axis digital accelerometer ([datasheet][adxl_manual]) over SPI interface of Raspberry Pi at sampling rates up to 3200 Hz. The output can be redirected to a standard output or CSV file.

- [Usage](#usage)
- [Installation](#installation)
- [Wiring](#wiring)
- [Testing](#testing)
- [Measuring vibration spectrums using FFT](#measuring-vibration-spectrums-using-fft)
- [SPI bus and sampling rates](#spi-bus-and-sampling-rates)

## Usage

Note: root privileges are required to run adxl345spi (i.e. `sudo adxl345spi [OPTIONS]`).

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

## Installation

```
sudo apt-get update
git clone https://github.com/nagimov/adxl345spi
cd adxl345spi
sudo make
sudo make install
```

Notes:

- root privileges are required by `make` in order to install dependencies (pigpio)
- root privileges are required by `make install` in order to install compiled binary to `/usr/local/bin`
- build is only tested on Raspbian / Raspbian Pi OS

## Wiring

Fritzing diagrams for two popular breadboards:

![adxl345spi_bb][adxl345spi_bb_png]

![adxl345spi_schem][adxl345spi_schem_png]

```
CS  : SPI_CE0_N (pin 24)
SDO : SPI_MISO (pin 21)
SDA : SPI_MOSI (pin 19)
SCL : SPI_CLK (pin 23)
```

Some ADXL345 breadboards have `VCC` pin marked as `3V3`.

## Testing

Accelerometer can be tested by running `adxl345spi` without arguments and verifying its output. Values of x, y and z acceleration should appear to be non-zero. Rotate the accelerometer around its x, y and z axis and observe acceleration values change accordingly from approximately -1.0 to +1.0.

## Measuring vibration spectrums using FFT

This simple setup demonstrates how to use ADXL345 accelerometer in order to measure vibration spectrums.

![adxl345_hdd_setup][adxl345_hdd_setup_jpg]

2.5" 5400 RPM laptop hard drive is used as source of vibrations. USB enclosure (a.k.a. "SATA to USB adapter") is used to power the hard drive from a USB port of Raspberry Pi. ADXL345 accelerometer board is mounted to the body of the hard drive using a C-clamp. **Care must be taken to prevent shorting or contacting exposed components on the accelerometer board or the hard drive**. Accelerometer is wired according to a wiring diagram.

Peak acceleration is expected to be measured at 90 Hz:
```
(5400 revolutions/min) / (60 sec/min) = 90 revolutions/sec = 90 Hz
```

A simple python script can be used to measure and display vibration spectrums:

```python
import os
import numpy as np
from matplotlib import mlab
import matplotlib.pyplot as plt
sample_rate_Hz = 3200
length_s = 2
os.system(f'sudo adxl345spi -t {length_s} -f {sample_rate_Hz} -s out.csv')
acc_data = np.genfromtxt('out.csv', delimiter=',', names=True)
acc_x, freq_x, _ = mlab.specgram(acc_data['x'], Fs=sample_rate_Hz, NFFT=sample_rate_Hz * length_s)
acc_y, freq_y, _ = mlab.specgram(acc_data['y'], Fs=sample_rate_Hz, NFFT=sample_rate_Hz * length_s)
acc_z, freq_z, _ = mlab.specgram(acc_data['z'], Fs=sample_rate_Hz, NFFT=sample_rate_Hz * length_s)
plt.plot(freq_x[10:], acc_x[10:], label='x', linewidth=0.5)
plt.plot(freq_y[10:], acc_y[10:], label='y', linewidth=0.5)
plt.plot(freq_z[10:], acc_z[10:], label='z', linewidth=0.5)
plt.yscale('log')
plt.xlim((0, 160))
plt.legend(loc='upper right')
plt.savefig('spectrum.png')
```

As expected, peak accelerations on all three axis are observed at 90 Hz:

![adxl345_hdd_fft][adxl345_hdd_fft_png]

## SPI bus and sampling rates

Due to limitations of I2C bus, it is impossible to achieve high sampling rates using I2C interface. SPI interface is used to achieve sampling rates up to 3200 samples per second (upper limit of ADXL345 chip itself). SPI baud rate is set to 2 Mbps, to ensure that no readings are lost during transmission.

For a standard output, downsampling is achieved by calling `sleep()` between transmissions, providing somewhat unstable sampling rate. If a reliable sampling rate is required, output to a file should be used.

For a file output, data is always read at a higher than required sampling rate (~30000 Hz for 2 Mbps SPI baud rate) and accurately downsampled to a specified value. Applications that require stable sampling rates and reliable time steps (e.g. spectrum analysis via FFT) should use a file output option.

[adxl_manual]: http://www.analog.com/en/products/mems/accelerometers/adxl345.html
[adxl345spi_bb_png]: img/adxl345spi_bb.png
[adxl345spi_schem_png]: img/adxl345spi_schem.png
[adxl345_hdd_setup_jpg]: img/adxl345_hdd_setup.jpg
[adxl345_hdd_fft_png]: img/adxl345_hdd_fft.png
