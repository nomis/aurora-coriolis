.PHONY: all clean upload fs uploadfs cleanfs htdocs src_htdocs pipenv
.DELETE_ON_ERROR:

# Don't emit native code because the ESP32 can't execute it from SPIRAM
MPY_CROSS_ARGS=-msmall-int-bits=31 -X emit=bytecode
PIPENV=$(CURDIR)/pipenv
PYTHON=$(PIPENV)/.venv/bin/python3

all:
	platformio run

native:
	platformio run -e native

clean: cleanfs
	+$(MAKE) -C $(PIPENV) -L clean
	rm -rf lib/micropython/port/build
	rm -rf .pio/*
	rm -f src/htdocs/*.gz.h
	+$(MAKE) -C micropython/mpy-cross clean
	+$(MAKE) -C app/pio/certs -L clean

upload:
	platformio run -t upload

uploadfs: fs
	platformio run -t uploadfs

htdocs: \
	$(patsubst %.xml,src/%.xml.gz.h,$(wildcard htdocs/*.xml))

data:
	mkdir -p data

fs: \
	$(patsubst %.py,data/%.mpy,$(wildcard scripts/*.py)) \
	$(patsubst %.json,data/%.cbor,$(wildcard buses/*.json)) \
	$(patsubst %.toml,data/%.cbor,$(wildcard buses/*.toml)) \
	$(patsubst %.yaml,data/%.cbor,$(wildcard buses/*.yaml)) \
	$(patsubst %.json,data/%.cbor,$(wildcard presets/*.json)) \
	$(patsubst %.toml,data/%.cbor,$(wildcard presets/*.toml)) \
	$(patsubst %.yaml,data/%.cbor,$(wildcard presets/*.yaml)) \
	$(patsubst %.json,data/%.cbor,$(wildcard profiles/*.json)) \
	$(patsubst %.toml,data/%.cbor,$(wildcard profiles/*.toml)) \
	$(patsubst %.yaml,data/%.cbor,$(wildcard profiles/*.yaml))

cleanfs:
	rm -rf data

micropython/mpy-cross/mpy-cross:
	+$(MAKE) -C micropython/mpy-cross

data/scripts: | data
	mkdir -p data/scripts

data/scripts/%.mpy: scripts/%.py | data/scripts micropython/mpy-cross/mpy-cross
	micropython/mpy-cross/mpy-cross $(MPY_CROSS_ARGS) -o $@ -s $(patsubst scripts/%,%,$<) $<

pipenv:
	+$(MAKE) -C $(PIPENV) -L

data/buses: | data
	mkdir -p data/buses

data/buses/%.cbor: buses/%.yaml | data/buses pipenv
	$(PYTHON) $(PIPENV)/yaml2cbor.py $< $@

data/buses/%.cbor: buses/%.toml | data/buses pipenv
	$(PYTHON) $(PIPENV)/toml2cbor.py $< $@

data/buses/%.cbor: buses/%.json | data/buses pipenv
	$(PYTHON) $(PIPENV)/json2cbor.py $< $@

data/profiles: | data
	mkdir -p data/profiles

data/profiles/%.cbor: profiles/%.yaml | data/profiles pipenv
	$(PYTHON) $(PIPENV)/yaml2cbor.py $< $@

data/profiles/%.cbor: profiles/%.toml | data/profiles pipenv
	$(PYTHON) $(PIPENV)/toml2cbor.py $< $@

data/profiles/%.cbor: profiles/%.json | data/profiles pipenv
	$(PYTHON) $(PIPENV)/json2cbor.py $< $@

data/presets: | data
	mkdir -p data/presets

data/presets/%.cbor: presets/%.yaml | data/presets pipenv
	$(PYTHON) $(PIPENV)/yaml2cbor.py $< $@

data/presets/%.cbor: presets/%.toml | data/presets pipenv
	$(PYTHON) $(PIPENV)/toml2cbor.py $< $@

data/presets/%.cbor: presets/%.json | data/presets pipenv
	$(PYTHON) $(PIPENV)/json2cbor.py $< $@

src_htdocs:
	mkdir -p src/htdocs

src/htdocs/%.gz.h: htdocs/% pipenv/gzip-embed.py | src_htdocs pipenv
	$(PYTHON) $(PIPENV)/gzip-embed.py $< $@
