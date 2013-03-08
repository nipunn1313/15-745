
int main() {
    int i, j, k;

    int l = 20;

    for (i = 0; i < 20; i++) {
        if (i > 100) {
            k = 10;     // Should not be moved out
        }
        else {
            k = 20;     // Should not be moved out
        }
        j = l + 2;
    }

    return (k + j);

}
