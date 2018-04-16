// To test:
// ~/cppcheck/cppcheck --dump misc-test.c && python ../misc.py -verify misc-test.c.dump

const char *a[] = {"a" "b"};
const char *b[] = {"a","b" "c"}; // stringConcatInArrayInit
const char *c[] = {
	"a\n"
	"a\n"
	"a\n"
	"a\n"
	"a\n"
	,
	"b\n"
	"b\n"
	"b\n"
	"b\n"
	"b\n"
};
