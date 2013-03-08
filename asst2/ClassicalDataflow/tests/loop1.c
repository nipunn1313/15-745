// A simple loop for testing out LICM 

int main() {
  int q = 0;
  int r = 5;
  for (int i=0; i<10; i++) {
    int x = 7*r + 22;   // This is loop invariant
    q = q + 5*i + 3*x + 5;
  }

  return q - 44;    // This can be checked for correctness
}
