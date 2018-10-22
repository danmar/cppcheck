// To test:
// ../../cppcheck --suppressions-list=suppressions.txt --dump misra-suppressions*-test.c && python ../misra.py misra-suppressions*-test.c.dump
// There should be no violations reported

// This needs to stay at line number 7 to make the test pass
// If it is changed update suppressions.txt with the new line number
#include <stdio.h> //21.6

extern int misra_5_2_var_hides_var______31x;
static int misra_5_2_var_hides_var______31y;//5.2
static int misra_5_2_function_hides_var_31x;
void misra_5_2_function_hides_var_31y(void) {}//5.2
void foo(void)
{
  int i;
  switch(misra_5_2_func1()) //16.4 16.6
  {
    case 1:
    {
      do
      {
        for(i = 0; i < 10; i++)
        {
          if(misra_5_2_func3()) //14.4
          {
            int misra_5_2_var_hides_var_1____31x;
            int misra_5_2_var_hides_var_1____31y;//5.2
          }
        }
      } while(misra_5_2_func2()); //14.4
    }
  }
}
