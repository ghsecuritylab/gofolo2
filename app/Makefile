
.PHONY: build flash flash_softdevice clean

build:
	make -C pca10040/s132/armgcc/
	nrfutil pkg generate --hw-version 52 --application-version 1 --application ./pca10040/s132/armgcc/_build/nrf52832_xxaa.hex --sd-req 0xAF --key-file private.key gofolo.zip

erase flash flash_softdevice clean:
	make -C pca10040/s132/armgcc/ $@
