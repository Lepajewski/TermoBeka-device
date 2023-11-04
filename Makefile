include make/platformio.mk

setup:
	sudo apt-get update \
	&& sudo apt-get -qqy upgrade

	$(SUDO) service udev restart

	make pio-setup

all: setup

.PHONY: setup all
