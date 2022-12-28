.PHONY: all clean upload fs uploadfs cleanfs pipenv
.DELETE_ON_ERROR:

# Don't emit native code because the ESP32 can't execute it from SPIRAM
MPY_CROSS_ARGS=-msmall-int-bits=31 -X emit=bytecode

PYTHON=pipenv/.venv/bin/python3

all:
	platformio run

native:
	platformio run -e native

clean: cleanfs
	platformio run -t clean
	+$(MAKE) -C micropython/mpy-cross clean
	rm -rf lib/micropython/port/build
	+$(MAKE) -C pipenv clean
	rm -rf .pio

upload:
	platformio run -t upload

uploadfs: fs
	platformio run -t uploadfs

data:
	mkdir -p data

fs: data/certs.ar \
	$(patsubst %.py,data/%.mpy,$(wildcard scripts/*.py)) \
	$(patsubst %.json,data/%.cbor,$(wildcard buses/*.json)) \
	$(patsubst %.yaml,data/%.cbor,$(wildcard buses/*.yaml)) \
	$(patsubst %.json,data/%.cbor,$(wildcard presets/*.json)) \
	$(patsubst %.yaml,data/%.cbor,$(wildcard presets/*.yaml)) \
	$(patsubst %.json,data/%.cbor,$(wildcard profiles/*.json)) \
	$(patsubst %.yaml,data/%.cbor,$(wildcard profiles/*.yaml))

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


pipenv:
	+$(MAKE) -C pipenv -L

data/buses: | data
	mkdir -p data/buses

data/buses/%.cbor: buses/%.yaml | data/buses pipenv
	$(PYTHON) pipenv/yaml2cbor.py $< $@

data/buses/%.cbor: buses/%.json | data/buses pipenv
	$(PYTHON) pipenv/json2cbor.py $< $@

data/profiles: | data
	mkdir -p data/profiles

data/profiles/%.cbor: profiles/%.yaml | data/profiles pipenv
	$(PYTHON) pipenv/yaml2cbor.py $< $@

data/profiles/%.cbor: profiles/%.json | data/profiles pipenv
	$(PYTHON) pipenv/json2cbor.py $< $@

data/presets: | data
	mkdir -p data/presets

data/presets/%.cbor: presets/%.yaml | data/presets pipenv
	$(PYTHON) pipenv/yaml2cbor.py $< $@

data/presets/%.cbor: presets/%.json | data/presets pipenv
	$(PYTHON) pipenv/json2cbor.py $< $@
