#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 1000000

int a[MAX_SIZE];
int b[MAX_SIZE];
int c[MAX_SIZE];

int main() {
    int i;

    i = 32;
    do {
        c[i] = c[i-32] + c[i-32];
        //c[i] = c[i-64] + c[i-128];
        i++;
    } while ( i < MAX_SIZE);
    return 0;
}
