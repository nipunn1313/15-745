#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 10000000

int a[MAX_SIZE];

int main() {
    int i;

    i = 32;
    do {
        a[i] = a[i-32] + a[i+32];
        i++;
    } while ( i + 32 < MAX_SIZE);
    return 0;
}
