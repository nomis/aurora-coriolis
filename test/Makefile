.PHONY: all native native_coverage coverage
.NOTPARALLEL:

DEFAULT_TARGETS:=native_coverage coverage
ifeq ($(CI),true)
DEFAULT_TARGETS:=native
endif

all: $(DEFAULT_TARGETS)

native:
	platformio test -d .. -e native_test

native_coverage:
	mkdir -p ../.pio/build/native_test_coverage/
	find ../.pio/build/native_test_coverage/ -type f -name '*.gcda' -exec rm {} \;
	platformio test -d .. -e native_test_coverage

coverage:
	lcov -d ../.pio/build/native_test_coverage/ -c -o ../.pio/build/native_test_coverage/coverage.info
	rm -rf ../coverage
	genhtml -o ../coverage/ --demangle-cpp --ignore-errors source ../.pio/build/native_test_coverage/coverage.info
