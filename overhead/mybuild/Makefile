.PHONY: build test clean test-%-success test-%-fail

.DEFAULT_GOAL := build

CURR_DIR := $(shell realpath ./)
PARALLEL_COUNT=$(shell nproc)

GEM5_ROOT = $(CURDIR)/..
M5_ISA=x86
M5OP_OBJ = $(GEM5_ROOT)/util/m5/build/$(M5_ISA)/out/libm5.a
EXAMPLE_CC=gcc
EXAMPLE_FLAGS= \
  -O0 -g \
  -I $(GEM5_ROOT)/include/ \
  -I $(GEM5_ROOT)/util/m5/src/ \
  -Wall \
  -Wextra \
  -fno-pie \
  -ggdb3 \
  -no-pie \
  -pedantic \
  -static

RUN_SIM=$(shell realpath ./run-gem5-fs.sh)
EXPECT_FAIL=./mybuild/expectfail.sh
TEST_FILES := $(wildcard ../tests/test-progs/interrupt/test-*.c)
TEST_NAMES := $(basename $(notdir $(TEST_FILES)))

# args for reun-gem5-fs.sh and their default values
debug_flags?=""
debug_file?=""

build-gem5:
	cd .. && scons build/X86/gem5.opt -j $(PARALLEL_COUNT) --ignore-style

build-gem5-debug:
	cd .. && scons build/X86/gem5.debug -j $(PARALLEL_COUNT) --ignore-style

.PRECIOUS: mybuildout/test-%

$(M5OP_OBJ):
	cd $(GEM5_ROOT) && scons -C util/m5 build/$(M5_ISA)/out/m5

mybuildout/test-%: $(M5OP_OBJ)
	mkdir -p mybuildout
	$(EXAMPLE_CC) $(GEM5_ROOT)/tests/test-progs/interrupt/$(subst mybuildout/,,$@).c $(M5OP_OBJ) $(EXAMPLE_FLAGS) -o $@

.SECONDEXPANSION:
test-%-success: mybuildout/$$@
	cd .. && $(RUN_SIM) $@ $(debug_flags) $(debug_file)

.SECONDEXPANSION:
test-%-fail: mybuildout/$$@
	cd .. && $(EXPECT_FAIL) $(RUN_SIM) $@ $(debug_flags) $(debug_file)


build-tests-serial: $(addprefix mybuildout/,$(TEST_NAMES))

build: build-gem5-debug

test: $(TEST_NAMES)
	@echo "!!!!!!!!!Tests successfully completed!!!!!!!!!"

clean-gem5:
	cd .. && scons -c

clean-tests:
	-rm -rf mybuildout

clean: clean-gem5 clean-tests
