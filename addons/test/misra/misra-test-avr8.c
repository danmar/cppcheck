// To test:
// ~/cppcheck/cppcheck --addon=misra --platform=avr8 misra-test-avr8.c

static void misra_10_4(void)
{
    // #10480
    char buf[1] = {'f'};
    const char c = '0';
    signed int x = buf[0] - c;
}

static void misra_12_2(void) {
  a = (((uint64_t)0xFF) << 32);
}

