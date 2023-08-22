// To test:
// ~/cppcheck/cppcheck--dump -DDUMMY --suppress=uninitvar --inline-suppr misra/misra-test-avr8.c --std=c89 --platform=avr8 && python3 ../misra.py -verify misra/misra-test-avr8.c.dump

static void misra_10_4(void)
{
    // #10480
    const char buf[1] = {'f'};
    const char c = '0';
    x = buf[0] - c;

    const char buf[2] = {0};
    x = 'a' == buf[0]; // no-warning

    typedef struct  {
      int t;
      char buf[2];
    } foo_t;
    const foo_t cmd = {0};
    x = 'b' == cmd.buf[0]; // no-warning

    const foo_t * pcmd = &cmd;
    x='c' == pcmd->buf[0]; // no-warning
    (void)cmd.t;
}

static void misra_12_2(void) {
  a = (((uint64_t)0xFF) << 32);
}

