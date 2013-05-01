#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 1000000

int a[MAX_SIZE];
int c[MAX_SIZE];

int main() {
    int i;

    i = 0;
    do {
        c[i] = a[i] + a[i+2];
        //c[i] = c[i-64] + c[i-128];
        i++;
    } while ( i + 2 < MAX_SIZE);
    return 0;
}
