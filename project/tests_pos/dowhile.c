#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 1000000

int foo(int* a, int* b, int* c) {
    int i;

    i = 0;
    do {
        c[i] = a[i] + b[i];
        i++;
    } while ( i < MAX_SIZE);
    return 0;
}
