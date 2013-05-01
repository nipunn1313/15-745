#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 10000000

int a[MAX_SIZE];

int main() {
    int i;

    i = 4;
    do {
        a[i] = a[i-4] + a[i+4];
        i++;
    } while ( i + 4 < MAX_SIZE);
    return 0;
}
