#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 1000000

int a[MAX_SIZE];
int b[MAX_SIZE];
int c[MAX_SIZE];

int main() {
    int i;

    i = 0;
    do {
        a[i] = a[i] + a[i+2];
        c[i] = a[i] + b[i];
        //c[i] = c[i-64] + c[i-128];
        i++;
    } while ( i < MAX_SIZE);
    return 0;
}
