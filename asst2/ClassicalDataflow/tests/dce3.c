int main() {
    int i, j, k, l;

    i = 0;      // Control dependency 
    j = 12;     // Data dependency

    if (i == 10) {
        k = j + 2;
        l = k + 3;  // Should be eliminated
    }
    else {
        k = j - 2;
        l = k - 3;  // Should be eliminated
    }

    return k;
    

}
