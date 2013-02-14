# Not real makefile -- just a substitute for shell script
# Compare the logs with disassembly (*.o.ll) to verify correctness

SHELL=bash

tests:
	
	clang -O0 -emit-llvm -c test1.c
	llvm-dis test1.o
	opt -load ../reaching-definitions.so -cd-reaching-definitions test1.o -o out >& reaching-definitions-test1.log
	
	clang -O0 -emit-llvm -c test2.c
	llvm-dis test2.o
	opt -load ../reaching-definitions.so -cd-reaching-definitions test2.o -o out >& reaching-definitions-test2.log
	
	clang -O0 -emit-llvm -c test3.c
	llvm-dis test3.o
	opt -load ../reaching-definitions.so -cd-reaching-definitions test3.o -o out >& reaching-definitions-test3.log
    
	clang -O0 -emit-llvm -c test1.c
	llvm-dis test1.o
	opt -load ../liveness.so -cd-liveness test1.o -o out >& liveness_test1.log
	
	clang -O0 -emit-llvm -c test2.c
	llvm-dis test2.o
	opt -load ../liveness.so -cd-liveness test2.o -o out >& liveness_test2.log
	
	clang -O0 -emit-llvm -c test3.c
	llvm-dis test3.o
	opt -load ../liveness.so -cd-liveness test3.o -o out >& liveness_test3.log

clean:
	rm *.o *.o.ll *.log out
