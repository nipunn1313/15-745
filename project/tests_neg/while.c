#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 1000000

int main() {
    int *a = (int *) malloc (4 * MAX_SIZE);
    int *b = (int *) malloc (4 * MAX_SIZE);
    int *c = (int *) malloc (4 * MAX_SIZE);

    int i;

    i = 0;
    while (i < MAX_SIZE) {
        c[i] = a[i] + b[i];
        i++;
    }
    return 0;
}
