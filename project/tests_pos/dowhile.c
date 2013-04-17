#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 1000000

int main() {
    int *a = (int *) malloc (4 * MAX_SIZE);
    int *b = (int *) malloc (4 * MAX_SIZE);
    int *c = (int *) malloc (4 * MAX_SIZE);

    int i;

    i = 0;
    do {
        c[i] = a[i] + b[i];
        i++;
    } while ( i < MAX_SIZE);
    return 0;
}
