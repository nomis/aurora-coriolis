BUILD := $(CURDIR)/build
QSTR_AUTOGEN_DISABLE := 0
CFLAGS := -I../../../micropython -I../../../include -I$(BUILD)
MICROPY_ROM_TEXT_COMPRESSION := 1
OBJ := all

include ../../../micropython/py/mkenv.mk
include $(TOP)/py/py.mk

SRC_MOD += ports/esp32/modutime.c
SRC_MOD += ../src/led_bus_udp.cpp
SRC_MOD += ../src/modaurcor.c
SRC_MOD += ../src/modulogging.c
SRC_MOD += ../src/py_module.cpp
SRC_MOD += ../src/ulogging.cpp

QSTR_DEFS += ../../../src/aurcor/qstrdefs.h

include $(TOP)/py/mkrules.mk
