.PHONY: all clean upload fs uploadfs

all:
	platformio run

native:
	platformio run -e native

clean:
	platformio run -t clean
	make -C lib/micropython/port clean
	rm -rf .pio
	rm -rf data

upload:
	platformio run -t upload

uploadfs: fs
	platformio run -t uploadfs

data:
	mkdir -p data

fs: data/certs.ar

data/certs.ar: certs/isrg-root-x1.der certs/isrg-root-x2.der | data
	ar q $@ $^
