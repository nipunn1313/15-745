#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 1000000

int a[MAX_SIZE];
int b[MAX_SIZE];
int c[MAX_SIZE];

int main() {
    int i;

    for (i = 0; i < MAX_SIZE; i++) {
        a[i] = i;
        b[i] = 2 * (i + 1);
    }

    i = 0;
    do {
        c[i] = a[i] + a[i+2];
        //c[i] = c[i-64] + c[i-128];
        i++;
    } while ( i + 2 < MAX_SIZE);

    
    for (i = 0; i + 2 < MAX_SIZE; i++) {
        if (c[i] != b[i]) {
            printf ("Incorrect value for i = %d\n", i);
            exit (1);
        }
    }

    return 0;
}
