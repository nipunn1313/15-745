all: dead-code-elimination.so loop-invariant-code-motion.so

CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) -g -O0 -Wall -Werror

dataflow.o: dataflow.cpp dataflow.h

%.so: %.o dataflow.o
	$(CXX) -dylib -flat_namespace -shared $^ -o $@

clean:
	rm -f *.o *~ *.so

run: dce1 dce2 dce3 licm1 licm2 licm3


dce1:   dead-code-elimination.so
	opt -mem2reg tests/dce1.o -o tests/dce1-m2r.o
	opt -load ./dead-code-elimination.so -mem2reg -cd-dce tests/dce1-m2r.o -o tests/dce1-out.o
	lli -stats -force-interpreter tests/dce1-m2r.o | tee tests/dce1-m2r.stats
	lli -stats -force-interpreter tests/dce1-out.o | tee tests/dce1-out.stats
	llvm-dis tests/dce1-m2r.o
	llvm-dis tests/dce1-out.o
	#**************************************************************
	# Compare tests/dce1-m2r.o.ll with  tests/dce1-out.o.ll
	# And    tests/dce1-m2r.stats with  tests/dce1-out.stats
	#**************************************************************

dce2:   dead-code-elimination.so
	opt -mem2reg tests/dce2.o -o tests/dce2-m2r.o
	opt -load ./dead-code-elimination.so -mem2reg -cd-dce tests/dce2-m2r.o -o tests/dce2-out.o
	lli -stats -force-interpreter tests/dce2-m2r.o | tee tests/dce2-m2r.stats
	lli -stats -force-interpreter tests/dce2-out.o | tee tests/dce2-out.stats
	llvm-dis tests/dce2-m2r.o
	llvm-dis tests/dce2-out.o
	#**************************************************************
	# Compare tests/dce2-m2r.o.ll with  tests/dce2-out.o.ll
	# And    tests/dce2-m2r.stats with  tests/dce2-out.stats
	#**************************************************************

dce3:   dead-code-elimination.so
	opt -mem2reg tests/dce3.o -o tests/dce3-m2r.o
	opt -load ./dead-code-elimination.so -mem2reg -cd-dce tests/dce3-m2r.o -o tests/dce3-out.o
	lli -stats -force-interpreter tests/dce3-m2r.o | tee tests/dce3-m2r.stats
	lli -stats -force-interpreter tests/dce3-out.o | tee tests/dce3-out.stats
	llvm-dis tests/dce3-m2r.o
	llvm-dis tests/dce3-out.o
	#**************************************************************
	# Compare tests/dce3-m2r.o.ll with  tests/dce3-out.o.ll
	# And    tests/dce3-m2r.stats with  tests/dce3-out.stats
	#**************************************************************

licm1:
	opt -mem2reg tests/licm1.o -o tests/licm1-m2r.o
	opt -load ./loop-invariant-code-motion.so -mem2reg -loop-simplify -cd-licm tests/licm1-m2r.o -o tests/licm1-out.o
	lli -stats -force-interpreter tests/licm1-m2r.o | tee tests/licm1-m2r.stats
	lli -stats -force-interpreter tests/licm1-out.o | tee tests/licm1-out.stats
	llvm-dis tests/licm1-m2r.o
	llvm-dis tests/licm1-out.o
	#**************************************************************
	# Compare tests/licm1-m2r.o.ll with  tests/licm1-out.o.ll
	# And    tests/licm1-m2r.stats with  tests/licm1-out.stats
	#**************************************************************

licm2:
	opt -mem2reg tests/licm2.o -o tests/licm2-m2r.o
	opt -load ./loop-invariant-code-motion.so -mem2reg -loop-simplify -cd-licm tests/licm2-m2r.o -o tests/licm2-out.o
	lli -stats -force-interpreter tests/licm2-m2r.o | tee tests/licm2-m2r.stats
	lli -stats -force-interpreter tests/licm2-out.o | tee tests/licm2-out.stats
	llvm-dis tests/licm2-m2r.o
	llvm-dis tests/licm2-out.o
	#**************************************************************
	# Compare tests/licm2-m2r.o.ll with  tests/licm2-out.o.ll
	# And    tests/licm2-m2r.stats with  tests/licm2-out.stats
	#**************************************************************

licm3:
	opt -mem2reg tests/licm3.o -o tests/licm3-m2r.o
	opt -load ./loop-invariant-code-motion.so -mem2reg -loop-simplify -cd-licm tests/licm3-m2r.o -o tests/licm3-out.o
	lli -stats -force-interpreter tests/licm3-m2r.o | tee tests/licm3-m2r.stats
	lli -stats -force-interpreter tests/licm3-out.o | tee tests/licm3-out.stats
	llvm-dis tests/licm3-m2r.o
	llvm-dis tests/licm3-out.o
	#**************************************************************
	# Compare tests/licm3-m2r.o.ll with  tests/licm3-out.o.ll
	# And    tests/licm3-m2r.stats with  tests/licm3-out.stats
	#**************************************************************



.PHONY: clean all
