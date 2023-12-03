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

cert-convert:
	@echo "#ifndef LIB_SERVER_MANAGER_CA_CERT_H_" > lib/server_manager/ca_cert.h
	@echo "#define LIB_SERVER_MANAGER_CA_CERT_H_\n\n" >> lib/server_manager/ca_cert.h
	xxd -i ca.crt >> lib/server_manager/ca_cert.h
	@echo "\n\n#endif  // LIB_SERVER_MANAGER_CA_CERT_H_" >> lib/server_manager/ca_cert.h


.PHONY: setup all count
