all: dead-code-elimination.so loop-invariant-code-motion.so

CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) -g -O0 -Wall -Werror

dataflow.o: dataflow.cpp dataflow.h

%.so: %.o dataflow.o
	$(CXX) -dylib -flat_namespace -shared $^ -o $@

clean:
	rm -f *.o *~ *.so

run:
	opt -load ./dead-code-elimination.so -cd-dce loop.o -o out

licm:
	opt -load ./loop-invariant-code-motion.so -mem2reg -loop-simplify -cd-licm tests/loop1.o -o out

.PHONY: clean all
