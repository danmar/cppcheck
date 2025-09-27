void foo(void) {
    union {
        int c;
        struct {
            char x;
            struct {
                char y;
            } s1;
        } s0;
    } u = {0};
}
