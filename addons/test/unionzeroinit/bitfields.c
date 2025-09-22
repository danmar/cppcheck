typedef union Evex {
	int u32;
	struct {
		char	mmm:3,
			b4:1,
			r4:1,
			b3:1,
			x3:1,
			r3:1;
	} extended;
} Evex;

void foo(void) {
    Evex evex = {0};
}
