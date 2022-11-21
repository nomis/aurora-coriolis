.PHONY: all native coverage
.NOTPARALLEL:

all: native coverage

native:
	mkdir -p ../.pio/build/native_test/
	find ../.pio/build/native_test/ -type f -name '*.gcda' -exec rm {} \;
	platformio test -d .. -e native_test

coverage:
	lcov -d ../.pio/build/native_test/ -c -o ../.pio/build/native_test/coverage.info
	rm -rf ../coverage
	genhtml -o ../coverage/ --demangle-cpp --ignore-errors source ../.pio/build/native_test/coverage.info
