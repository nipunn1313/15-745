#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 1000000

int main() {
//    int a[MAX_SIZE];
//    int b[MAX_SIZE];
//    int c[MAX_SIZE];
    int *a = (int *) malloc (4 * MAX_SIZE);
    int *b = (int *) malloc (4 * MAX_SIZE);
    int *c = (int *) malloc (4 * MAX_SIZE);

    int i;

//    for (i = 0; i < MAX_SIZE; i++) {
//        a[i] = i;
//        b[i] = 2 * i;
//        c[i] = 0;
//    }

    i = 0;
    do {
//        a[i] = 3 + a[i];
//        b[i] = b[i] - 2;
        a[i] = a[i] + b[i];
        c[i] = a[i] + b[i];
        i++;
    } while ( i < MAX_SIZE);
    return 0;
}
