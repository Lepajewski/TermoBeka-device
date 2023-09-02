include make/common.mk

ENV ?= $(DEFAULT_ENV)

pio-setup:
	mkdir -p ~/.local/bin
	mkdir -p /usr/local/bin
	ln -sf ~/.platformio/penv/bin/platformio ~/.local/bin/platformio
	ln -sf ~/.platformio/penv/bin/pio ~/.local/bin/pio
	ln -sf ~/.platformio/penv/bin/piodebuggdb ~/.local/bin/piodebuggdb

	$(SUDO) chmod a+rw /dev/ttyUSB0

pio-clean:
	platformio run --target clean

pio-fullclean:
	platformio run --target fullclean

pio-build:
	platformio run --environment $(ENV)

pio-flash-erase:
	platformio run --target erase --environment $(ENV)

pio-flash:
	$(SUDO) chmod a+rw /dev/ttyUSB0
	platformio run --target upload --environment $(ENV)

pio-monitor:
	platformio device monitor --environment $(ENV)

pio-menuconfig:
	platformio run --target menuconfig --environment $(ENV)

.PHONY: pio-setup pio-clean pio-fullclean pio-build pio-flash pio-monitor pio-menuconfig
