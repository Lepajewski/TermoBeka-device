include make/platformio.mk

setup:
	sudo apt-get update \
	&& sudo apt-get -qqy upgrade

	git submodule init
	git submodule update

	make pio-setup

all: setup

.PHONY: setup all
