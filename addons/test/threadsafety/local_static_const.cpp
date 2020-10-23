struct Dummy {
 int x;
};
void func() {
  static const Dummy dummy;
}
