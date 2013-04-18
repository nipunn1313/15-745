#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 1000000

int foo(int* a, int* b, int* c, int len) {
    int i;

    i = 0;
    do {
        c[i] = c[i-24] + c[i-16];
        i++;
    } while ( i < len);
    return 0;
}
