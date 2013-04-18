int foo(int *aa, int *bb, int SIZE) {
  int count = 190;
#if 1
  for(int i=0;i<SIZE;i++) {
    aa[i] = bb[i] + count;
    count++;
  }
#endif
#if 0
  for(int i=SIZE;i>0;i--) {
    aa[i] = bb[i] + count;
    count++;
  }
#endif
}
