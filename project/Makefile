#all: dead-code-elimination.so loop-invariant-code-motion.so
#
#dataflow.o: dataflow.cpp dataflow.h
#
#CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) -g -O0 -Wall -Werror
#
#%.so: %.o dataflow.o
#	$(CXX) -dylib -flat_namespace -shared $^ -o $@
#

SHELL := bash


#all: loop_pass.so
all: LoopVectorize.so


CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) -g -O0 -Wall -Werror

%.so: %.o
	$(CXX) -dylib -flat_namespace -shared $^ -o $@



run: 
	./run_simd.sh |& tee run_simd.log

%.o.bc: %.c
	clang -O0 -emit-llvm -o $@ -c $^

simd.o.opt.bc: simd.o.bc
	cp $^ simd.o.opt.bc

	opt -mem2reg -o simd.m2r.o.bc simd.o.opt.bc
	cp simd.m2r.o.bc simd.o.opt.bc

	opt -debug -loop-unroll -o simd.m2r.loop.o.bc simd.o.opt.bc
	cp simd.m2r.loop.o.bc simd.o.opt.bc

	opt -debug -loop-simplify -loop-vectorize -o simd.m2r.vect.o.bc simd.o.opt.bc
	cp simd.m2r.vect.o.bc simd.o.opt.bc

	opt -debug -load ./loop-invariant-code-motion.so -cd-licm -o simd.licm.o.bc simd.o.opt.bc
	cp simd.m2r.licm.o.bc simd.o.opt.bc

#	opt -debug -licm -o simd.m2r.licm.o.bc simd.o.opt.bc
#	cp simd.m2r.licm.o.bc simd.o.opt.bc
#
#	opt -debug -vectorize -o simd.m2r.vect.o.bc simd.o.opt.bc
#	cp simd.m2r.vect.o.bc simd.o.opt.bc

#	cp simd.o.opt temp.o
#	opt -load ./loop-invariant-code-motion.so -cd-licm -o simd.o.opt temp.o 
#
#	
#
#run: simd.o.opt.bc
#	lli -stats -force-interpreter simd.o.bc
#	lli -stats -force-interpreter simd.m2r.o.bc
#	lli -stats -force-interpreter simd.m2r.vect.o.bc
#
#	
#CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) -g -O0 -Wall -Werror
#
#dataflow.o: dataflow.cpp dataflow.h
#
#%.so: %.o dataflow.o
#	$(CXX) -dylib -flat_namespace -shared $^ -o $@
#
clean:
	rm -f *.o *~ *.so *.bc *.ll unoptimized_simd optimized_simd

.PHONY: clean all
