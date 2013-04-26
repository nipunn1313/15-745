#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 1000000

int foo(int* a, int* b, int* c, int len) {
    int i;

    i = 0;
    do {
        c[i] = c[i-8] + c[i+32];
        //c[i] = c[i-64] + c[i-128];
        i++;
    } while ( i < len);
    return 0;
}
