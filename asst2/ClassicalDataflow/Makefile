all: dead-code-elimination.so loop-invariant-code-motion.so

CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) -g -O0 -Wall -Werror

dataflow.o: dataflow.cpp dataflow.h

%.so: %.o dataflow.o
	$(CXX) -dylib -flat_namespace -shared $^ -o $@

clean:
	rm -f *.o *~ *.so

run:
	opt -load ./dead-code-elimination.so -cd-dce loop.o -o out

dce1:
	opt -mem2reg tests/dce1.o -o tests/dce1-m2r.o
	opt -load ./dead-code-elimination.so -mem2reg -cd-dce tests/dce1-m2r.o -o out

licm1:
	opt -mem2reg tests/loop1.o -o tests/loop1-m2r.o
	opt -load ./loop-invariant-code-motion.so -mem2reg -loop-simplify -cd-licm tests/loop1-m2r.o -o out

licm2:
	opt -mem2reg tests/loop2.o -o tests/loop2-m2r.o
	opt -load ./loop-invariant-code-motion.so -mem2reg -loop-simplify -cd-licm tests/loop2-m2r.o -o out

.PHONY: clean all
