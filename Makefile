.PHONY: all clean upload fs uploadfs cleanfs

# Don't emit native code because the ESP32 can't execute it from SPIRAM
MPY_CROSS_ARGS=-msmall-int-bits=31 -X emit=bytecode

all:
	platformio run

native:
	platformio run -e native

clean: cleanfs
	platformio run -t clean
	+$(MAKE) -C micropython/mpy-cross clean
	rm -rf lib/micropython/port/build
	rm -rf .pio

upload:
	platformio run -t upload

uploadfs: fs
	platformio run -t uploadfs

data:
	mkdir -p data

fs: data/certs.ar $(patsubst %.py,data/%.mpy,$(wildcard scripts/*.py))

cleanfs:
	rm -rf data

data/certs.ar: certs/isrg-root-x1.der certs/isrg-root-x2.der | data
	ar q $@ $^

micropython/mpy-cross/mpy-cross:
	+$(MAKE) -C micropython/mpy-cross

data/scripts: | data
	mkdir -p data/scripts

data/scripts/%.mpy: scripts/%.py | data/scripts micropython/mpy-cross/mpy-cross
	micropython/mpy-cross/mpy-cross $(MPY_CROSS_ARGS) -o $@ -s $(patsubst scripts/%,%,$<) $<
