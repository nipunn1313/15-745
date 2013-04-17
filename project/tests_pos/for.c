#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 1000000

int foo(int* a, int* b, int* c) {
    int i;

    for (i = 0; i < MAX_SIZE; i++) {
        c[i] = a[i] + b[i];
    };
    return 0;
}
