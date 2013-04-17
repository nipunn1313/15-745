#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 1000000

int main() {
    int *a = (int *) malloc (4 * MAX_SIZE);
    int *b = (int *) malloc (4 * MAX_SIZE);
    int *c = (int *) malloc (4 * MAX_SIZE);

    int i;

    for (i = 0; i < MAX_SIZE; i++) {
        c[i] = a[i] + b[i];
    };
    return 0;
}
