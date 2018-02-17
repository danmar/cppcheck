
// complexity is measured according to the "shortcut" here:
// https://stackoverflow.com/questions/9097987/calculation-of-cyclomatic-complexity

void test1(int x) { // lines:2 complexity:3
  if (x>0) {}
  if (x<10) {}
}

void test2(int x) { // lines:1 complexity:3
    if (x>0 && x<10) {}
}
