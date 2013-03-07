int main() {
  int q = 2;
  int r = 5;
  for (int i=0; i<10; i++) {
    int x = 7*r + 22;
    q = q + 5*i + 3*x + 5;
  }

  return q - 44;
}
