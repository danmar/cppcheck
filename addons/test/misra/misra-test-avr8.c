// To test:
// ~/cppcheck/cppcheck --addon=misra --platform=avr8 misra-test-avr8.c

static void misra_12_2(void) {
  a = (((uint64_t)0xFF) << 32);
}

