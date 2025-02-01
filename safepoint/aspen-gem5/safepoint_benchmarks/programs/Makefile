LLVM_VERSION=11

CC=clang-$(LLVM_VERSION)
OPT=opt-$(LLVM_VERSION)
CXX=clang++-$(LLVM_VERSION)

CFLAGS=-O3 -g -Wall -pthread -lm
CXXFLAGS=-std=c++11 -O3 -g -Wall
PASS_FLAGS=-c -emit-llvm

SRCS=$(wildcard *.c)
BCS=$(patsubst %.c,build/%.bc,$(SRCS))
BCS_OPT=$(patsubst %.c,build/%.opt.bc,$(SRCS))
BCS_PASS=$(patsubst %.c,build/%.pass.opt.bc,$(SRCS))
BCS_FPASS=$(patsubst %.c,build/%.fpass.opt.bc,$(SRCS))
BCS_FLPASS=$(patsubst %.c,build/%.flpass.opt.bc,$(SRCS))
BCS_SAFEPOINT=$(patsubst %.c,build/%.safepoint.opt.bc,$(SRCS))
BINS=$(patsubst %.c,build/%_base,$(SRCS))
BINS_SIGNAL=$(patsubst %.c,build/%_signal,$(SRCS))
BINS_UINTR=$(patsubst %.c,build/%_uintr,$(SRCS))
BINS_CONCORD=$(patsubst %.c,build/%_concord,$(SRCS))
BINS_CONCORDF=$(patsubst %.c,build/%_concordf,$(SRCS))
BINS_CONCORDFL=$(patsubst %.c,build/%_concordfl,$(SRCS))
BINS_SAFEPOINT=$(patsubst %.c,build/%_safepoint,$(SRCS))

BCS_UNROLL=$(patsubst %.c,build/%_unroll.bc,$(SRCS))
BCS_OPT_UNROLL=$(patsubst %.c,build/%_unroll.opt.bc,$(SRCS))
BCS_PASS_UNROLL=$(patsubst %.c,build/%_unroll.pass.opt.bc,$(SRCS))
BINS_UNROLL=$(patsubst %.c,build/%_unroll,$(SRCS))
BINS_CONCORD_UNROLL=$(patsubst %.c,build/%_concord_unroll,$(SRCS))

# GAPS=pr cc bc bfs
# GAP_BCS=$(patsubst %,cxxbuild/%.bc,$(GAPS))
# GAP_BCS_OPT=$(patsubst %,cxxbuild/%.opt.bc,$(GAPS))
# GAP_BCS_PASS=$(patsubst %,cxxbuild/%.pass.opt.bc,$(GAPS))
# GAP_BCS_FPASS=$(patsubst %,cxxbuild/%.fpass.opt.bc,$(GAPS))
# GAP_BCS_FLPASS=$(patsubst %,cxxbuild/%.flpass.opt.bc,$(GAPS))
# GAP_BINS=$(patsubst %,cxxbuild/%,$(GAPS))
# GAP_BINS_SIGNAL=$(patsubst %,cxxbuild/%_signal,$(GAPS))
# GAP_BINS_UINTR=$(patsubst %,cxxbuild/%_uintr,$(GAPS))
# GAP_BINS_CONCORD=$(patsubst %,cxxbuild/%_concord,$(GAPS))
# GAP_BINS_CONCORDF=$(patsubst %,cxxbuild/%_concordf,$(GAPS))
# GAP_BINS_CONCORDFL=$(patsubst %,cxxbuild/%_concordfl,$(GAPS))

# GAP_BCS_UNROLL=$(patsubst %,cxxbuild/%_unroll.bc,$(GAPS))
# GAP_BCS_OPT_UNROLL=$(patsubst %,cxxbuild/%_unroll.opt.bc,$(GAPS))
# GAP_BCS_PASS_UNROLL=$(patsubst %,cxxbuild/%_unroll.pass.opt.bc,$(GAPS))
# GAP_BINS_UNROLL=$(patsubst %,cxxbuild/%_unroll,$(GAPS))
# GAP_BINS_CONCORD_UNROLL=$(patsubst %,cxxbuild/%_concord_unroll,$(GAPS))

# LIBMCF=mcf/src/libmcf.a 
# LIBMCF_UNROLL=mcf_unroll/src/libmcf.a 
# LIBMCF_CONCORD=mcf/src/libmcf_concord.a
# LIBMCF_CONCORDF=mcf/src/libmcf_concordf.a
# LIBMCF_CONCORDFL=mcf/src/libmcf_concordfl.a
# LIBMCF_CONCORD_UNROLL=mcf_unroll/src/libmcf_concord.a
# BINMCF=build/mcf
# BINMCF_UNROLL=build/mcf_unroll
# BINMCF_SIGNAL=build/mcf_signal
# BINMCF_UINTR=build/mcf_uintr
# BINMCF_CONCORD=build/mcf_concord
# BINMCF_CONCORDF=build/mcf_concordf
# BINMCF_CONCORDFL=build/mcf_concordfl
# BINMCF_CONCORD_UNROLL=build/mcf_concord_unroll

ROOT_DIR=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

TIMER_NONE=$(ROOT_DIR)/../timer_base.o $(ROOT_DIR)/../m5/libm5.a
TIMER_SIGNAL=$(ROOT_DIR)/../timer_signal.o $(ROOT_DIR)/../m5/libm5.a
TIMER_UINTR=$(ROOT_DIR)/../timer_uintr.o $(ROOT_DIR)/../m5/libm5.a
TIMER_CONCORD=$(ROOT_DIR)/../timer_concord.o $(ROOT_DIR)/../m5/libm5.a
TIMER_SAFEPOINT=$(ROOT_DIR)/../timer_safepoint.o $(ROOT_DIR)/../m5/libm5.a

OPT_CONFIG=-postdomtree -mem2reg -indvars -loop-simplify -branch-prob -scalar-evolution 
CONCORD_PASS=$(ROOT_DIR)/../concord/src/cache-line-pass/build/src/libConcordPass.so
CONCORDF_PASS=$(ROOT_DIR)/../concord/src/cache-line-pass-func/build/src/libConcordPass.so
CONCORDFL_PASS=$(ROOT_DIR)/../concord/src/cache-line-pass-loopfunc/build/src/libConcordPass.so
SAFEPOINT_PASS=$(ROOT_DIR)/../concord/src/safepoint/build/src/libConcordPass.so

c_target=$(BCS) $(BCS_OPT) $(BCS_PASS) $(BCS_FPASS) $(BCS_FLPASS) $(BCS_SAFEPOINT) \
	$(BINS) $(BINS_SIGNAL) $(BINS_UINTR) $(BINS_CONCORD) $(BINS_CONCORDF) $(BINS_CONCORDFL) $(BINS_SAFEPOINT) \
	$(BCS_UNROLL) $(BCS_OPT_UNROLL) $(BCS_PASS_UNROLL) $(BINS_UNROLL) $(BINS_CONCORD_UNROLL) \

cxx_target=$(GAP_BCS) $(GAP_BCS_OPT) $(GAP_BCS_PASS) $(GAP_BCS_FPASS) $(GAP_BCS_FLPASS) \
	$(GAP_BINS) $(GAP_BINS_SIGNAL) $(GAP_BINS_UINTR) $(GAP_BINS_CONCORD) $(GAP_BINS_CONCORDF) $(GAP_BINS_CONCORDFL) \
	$(GAP_BCS_UNROLL) $(GAP_BCS_OPT_UNROLL) $(GAP_BCS_PASS_UNROLL) $(GAP_BINS_UNROLL) $(GAP_BINS_CONCORD_UNROLL) \

mcf_target=$(BINMCF) $(BINMCF_SIGNAL) $(BINMCF_UINTR) $(BINMCF_CONCORD) $(BINMCF_CONCORDF) $(BINMCF_CONCORDFL) \
			$(BINMCF_UNROLL) $(BINMCF_CONCORD_UNROLL)

all: $(c_target)
	
# For C:
build/%_base: build/%.bc
	$(CC) $(CFLAGS) -o $@ $(TIMER_NONE) $<

build/%_signal: build/%.bc
	$(CC) $(CFLAGS) -o $@ $(TIMER_SIGNAL) $<

build/%_uintr: build/%.opt.bc
	$(CC) $(CFLAGS) -o $@ $(TIMER_UINTR) $<

build/%_concord: build/%.pass.opt.bc
	$(CC) $(CFLAGS) -o $@ $(TIMER_CONCORD) $<

build/%_concordf: build/%.fpass.opt.bc
	$(CC) $(CFLAGS) -o $@ $(TIMER_CONCORD) $<

build/%_concordfl: build/%.flpass.opt.bc
	$(CC) $(CFLAGS) -o $@ $(TIMER_CONCORD) $<

build/%_safepoint: build/%.safepoint.opt.bc
	$(CC) $(CFLAGS) -o $@ $(TIMER_SAFEPOINT) $<

build/%.pass.opt.bc: build/%.opt.bc
	$(OPT) -S -load $(CONCORD_PASS) -yield -o $@ $<

build/%.fpass.opt.bc: build/%.opt.bc
	$(OPT) -S -load $(CONCORDF_PASS) -yield -o $@ $<

build/%.flpass.opt.bc: build/%.opt.bc
	$(OPT) -S -load $(CONCORDFL_PASS) -yield -o $@ $<

build/%.safepoint.opt.bc: build/%.opt.bc
	$(OPT) -S -load $(SAFEPOINT_PASS) -yield -o $@ $<

build/%.opt.bc: build/%.bc
	$(OPT) -S $(OPT_CONFIG) -o $@ $<

build/%.bc: %.c
	$(CC) $(CFLAGS) $(PASS_FLAGS) -c $< -o $@

build/%_unroll: build/%_unroll.bc
	$(CC) $(CFLAGS) -o $@ $(TIMER_NONE) $<

build/%_concord_unroll: build/%_unroll.pass.opt.bc
	$(CC) $(CFLAGS) -o $@ $(TIMER_CONCORD) $<

build/%_unroll.pass.opt.bc: build/%_unroll.opt.bc
	$(OPT) -S -load $(CONCORD_PASS) -yield -o $@ $<

build/%_unroll.opt.bc: build/%_unroll.bc
	$(OPT) -S $(OPT_CONFIG) -o $@ $<

build/%_unroll.bc: %.c
	$(CC) $(CFLAGS) -DUNROLL $(PASS_FLAGS) -c $< -o $@


# # For CXX:
# cxxbuild/%: cxxbuild/%.bc
# 	$(CXX) $(CXXFLAGS) -o $@ $(TIMER_NONE) $<

# cxxbuild/%_signal: cxxbuild/%.bc
# 	$(CXX) $(CXXFLAGS) -o $@ $(TIMER_SIGNAL) $<

# cxxbuild/%_uintr: cxxbuild/%.opt.bc
# 	$(CXX) $(CXXFLAGS) -o $@ $(TIMER_UINTR) $<

# cxxbuild/%_concord: cxxbuild/%.pass.opt.bc
# 	$(CXX) $(CXXFLAGS) -o $@ $(TIMER_CONCORD) $<

# cxxbuild/%_concordf: cxxbuild/%.fpass.opt.bc
# 	$(CXX) $(CXXFLAGS) -o $@ $(TIMER_CONCORD) $<

# cxxbuild/%_concordfl: cxxbuild/%.flpass.opt.bc
# 	$(CXX) $(CXXFLAGS) -o $@ $(TIMER_CONCORD) $<

# cxxbuild/%.pass.opt.bc: cxxbuild/%.opt.bc
# 	$(OPT) -S -load $(CONCORD_PASS) -yield -o $@ $<

# cxxbuild/%.fpass.opt.bc: cxxbuild/%.opt.bc
# 	$(OPT) -S -load $(CONCORDF_PASS) -yield -o $@ $<

# cxxbuild/%.flpass.opt.bc: cxxbuild/%.opt.bc
# 	$(OPT) -S -load $(CONCORDFL_PASS) -yield -o $@ $<

# cxxbuild/%.opt.bc: cxxbuild/%.bc
# 	$(OPT) -S $(OPT_CONFIG) -o $@ $<

# cxxbuild/%.bc: gapbs/src/%.cc gapbs/src/*.h
# 	$(CXX) $(CXXFLAGS) $(PASS_FLAGS) -I. $< -o $@ 

# cxxbuild/%_unroll: cxxbuild/%_unroll.bc
# 	$(CXX) $(CXXFLAGS) -o $@ $(TIMER_NONE) $<

# cxxbuild/%_concord_unroll: cxxbuild/%_unroll.pass.opt.bc
# 	$(CXX) $(CXXFLAGS) -o $@ $(TIMER_CONCORD) $<

# cxxbuild/%_unroll.pass.opt.bc: cxxbuild/%_unroll.opt.bc
# 	$(OPT) -S -load $(CONCORD_PASS) -yield -o $@ $<

# cxxbuild/%_unroll.opt.bc: cxxbuild/%_unroll.bc
# 	$(OPT) -S $(OPT_CONFIG) -o $@ $<

# cxxbuild/%_unroll.bc: gapbs/src/%.cc gapbs/src/*.h
# 	$(CXX) $(CXXFLAGS) -DUNROLL $(PASS_FLAGS) -I. $< -o $@ 


# # For mcf:
# $(BINMCF): $(LIBMCF)
# 	$(CC) $(CFLAGS) -o $@ $(TIMER_NONE) $<

# $(BINMCF_SIGNAL): $(LIBMCF)
# 	$(CC) $(CFLAGS) -o $@ $(TIMER_SIGNAL) $<

# $(BINMCF_UINTR): $(LIBMCF)
# 	$(CC) $(CFLAGS) -o $@ $(TIMER_UINTR) $<

# $(BINMCF_CONCORD): $(LIBMCF_CONCORD)
# 	$(CC) $(CFLAGS) -o $@ $(TIMER_CONCORD) $<

# $(BINMCF_CONCORDF): $(LIBMCF_CONCORDF)
# 	$(CC) $(CFLAGS) -o $@ $(TIMER_CONCORD) $<
	
# $(BINMCF_CONCORDFL): $(LIBMCF_CONCORDFL)
# 	$(CC) $(CFLAGS) -o $@ $(TIMER_CONCORD) $<

# $(BINMCF_UNROLL): $(LIBMCF_UNROLL)
# 	$(CC) $(CFLAGS) -o $@ $(TIMER_NONE) $<

# $(BINMCF_CONCORD_UNROLL): $(LIBMCF_CONCORD_UNROLL)
# 	$(CC) $(CFLAGS) -o $@ $(TIMER_CONCORD) $<


clean: 
	rm $(c_target)


# clean:
# 	rm  $(BCS) $(BCS_OPT) $(BCS_PASS) $(BCS_FPASS) \
# 		$(BINS) $(BINS_SIGNAL) $(BINS_UINTR) $(BINS_CONCORD) $(BINS_CONCORDF) \
# 		$(BCS_UNROLL) $(BCS_OPT_UNROLL) $(BCS_PASS_UNROLL) $(BINS_UNROLL) $(BINS_CONCORD_UNROLL) \
# 		$(GAP_BCS) $(GAP_BCS_OPT) $(GAP_BCS_PASS) $(GAP_BCS_FPASS) \
# 		$(GAP_BINS) $(GAP_BINS_SIGNAL) $(GAP_BINS_UINTR) $(GAP_BINS_CONCORD) $(GAP_BINS_CONCORDF) \
# 		$(GAP_BCS_UNROLL) $(GAP_BCS_OPT_UNROLL) $(GAP_BCS_PASS_UNROLL) $(GAP_BINS_UNROLL) $(GAP_BINS_CONCORD_UNROLL) \
# 		$(BINMCF) $(BINMCF_SIGNAL) $(BINMCF_UINTR) $(BINMCF_CONCORD) $(BINMCF_UNROLL) $(BINMCF_CONCORD_UNROLL)