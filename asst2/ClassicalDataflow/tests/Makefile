# Compare the logs with disassembly (*.o.ll) to verify correctness

SHELL=bash

all: licm1.o licm2.o licm3.o dce1.o dce2.o dce3.o

%.o : %.c
	clang -O0 -emit-llvm -c $^ -o $@

clean:
	rm *.o *.o.ll *.log out *.stats
