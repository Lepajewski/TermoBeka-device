include make/platformio.mk

setup:
	sudo apt-get update \
	&& sudo apt-get -qqy upgrade

	$(SUDO) service udev restart

	make pio-setup

all: setup

LIB_LINES := $(shell find $(LIB_DIR) -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.h" \) -exec cat {} + | wc -l)
SRC_LINES := $(shell find $(SRC_DIR) -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.h" \) -exec cat {} + | wc -l)

count:
	@echo "Lines total: $$(( $(LIB_LINES) + $(SRC_LINES) ))"

.PHONY: setup all count
