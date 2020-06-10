CC = gcc
CFLAGS = -Wall -pthread -lpigpio -lrt
INSTALL = `which install`

packages:
	if ! dpkg-query -W -f='$${Status}' pigpio | grep "ok installed"; then apt-get -y install pigpio; fi

.PHONY: packages

adxl345spi: adxl345spi.c
	$(CC) $(CFLAGS) adxl345spi.c -o adxl345spi

install: adxl345spi
	$(INSTALL) ./adxl345spi /usr/local/bin/adxl345spi

clean:
	rm -f adxl345spi
