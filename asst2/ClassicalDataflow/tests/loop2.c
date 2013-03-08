#include <stdio.h>

int main () {
    int i, j, k;

    int result = 0;
    int result1 = 0;

    for (i = 0; i < 5; i++) {
        result1 = 5;    // Loop invariant
        for (j = 0; j < 10; j++) {
            k = 10 + result1;   // Loop invariant
            result += 4 * k;
        }
    }
    
    printf ("Loop2 result : %d\n", result);

    return result;
}
