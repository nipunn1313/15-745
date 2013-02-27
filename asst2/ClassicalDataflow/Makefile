all: dead-code-elimination.so

CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) -g -O0 -Wall -Werror

dataflow.o: dataflow.cpp dataflow.h

%.so: %.o dataflow.o
	$(CXX) -dylib -flat_namespace -shared $^ -o $@

clean:
	rm -f *.o *~ *.so

run:
	opt -load ./dead-code-elimination.so -cd-dce loop.o -o out

.PHONY: clean all
