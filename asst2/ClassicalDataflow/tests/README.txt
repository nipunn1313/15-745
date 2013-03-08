
1. Run make to compile and make clean to clean.

2. Following is the intent of dead-code-elimination tests :
    dce1.c :  Three variables i, j, k are used and j is returned
              return statement make all assignment to j live. 
              Value of i is used by j so those become live.
              k uses j but does not affect j, so assignments to k should be dead

    dce2.c :  Different kind of side effect is considered. A value is printed 
              and another value is returned so their dependence should become live

    dce3.c : Control dependence is also considered. Variables which use the value 
             of returned variable remains dead

3. Following is the intent of loop-invariant-code-motion tests :

    licm1.c : The variable x is assigned within loop but uses outer variables
              it should be detected as loop invariant

    licm2.c : Nesting of loop and ordering is tested.
              Value of result printed will be changed if ordering is interchanged.

    licm3.c : Multiple branches assign to same variable tests - it should not be moved out
              A normal variable also kept to ensure it is moved out
              
