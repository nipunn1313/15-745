# Compare the logs with disassembly (*.o.ll) to verify correctness

SHELL=bash

all: loop1.o loop2.o dce1.o

%.o : %.c
	clang -O0 -emit-llvm -c $^ -o $@

clean:
	rm *.o *.o.ll *.log out
