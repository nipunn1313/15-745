#include <stdio.h>

int main() {
    int i, j, k;
    i = 0;
    j = 1 + i;
    k = 3 + i;

    // K has side effect of printing
    printf ("k is %d\n", k);

    return i;

    j = 20;
    return j;
}
