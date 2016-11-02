
int TestData[10];
#define TEST(DATA)   DATA=1000;TestData[DATA]=0;  DATA=0;TestData[0]=100/(DATA);

int global;
void test_global() {
  TEST(global);
}

int global_array[10];
void test_global_array() {
  TEST(global_array[3]);
}

int *global_pointer;
void test_global_pointer() {
  TEST(*global_pointer);
}


void test_local() {
  int local;
  TEST(local);
}

void test_local_array() {
  int local_array[10];
  TEST(local_array[3]);
}

void test_local_pointer() {
  int local;
  int *local_pointer = &local;
  TEST(*local_pointer);
}


struct S {
  int member;
};
void test_struct_member(struct S *s) {
  TEST(s->member);
}


