// To test:
// ../../cppcheck --suppressions-list=suppressions.txt --dump misra-suppressions*-test.c && python ../misra.py misra-suppressions*-test.c.dump
// There should be no violations reported

union misra_5_2_field_hides_field__63x { //19.2
int misra_5_2_field_hides_field__31x;
int misra_5_2_field_hides_field__31y;//5.2
};
struct misra_5_2_field_hides_field__63y { //5.2
int misra_5_2_field_hides_field1_31x;
int misra_5_2_field_hides_field1_31y;//5.2
};
const char *s41_1 = "\x41g"; // 4.1
const char *s41_2 = "\x41\x42";
