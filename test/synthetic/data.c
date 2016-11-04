
int TestData[10];

int global;
void global() {
  global = 1000;
  TestData[global] = 0; // BUG
}

int global_array[10];
void global_array() {
  global_array[3] = 1000;
  TestData[global_array[3]] = 0; // BUG
}

int *global_pointer;
void global_pointer() {
  *global_pointer = 1000;
  TestData[*global_pointer] = 0; // BUG
}


void local() {
  int local;
  local = 1000;
  TestData[local] = 0; // BUG
}

void local_array() {
  int local_array[10];
  local_array[3] = 1000;
  TestData[local_array[3]] = 0; // BUG
}

void local_alias_1() {
  int local;
  int *local_alias = &local;
  *local_alias = 1000;
  TestData[*local_alias] = 0; // BUG
}

void local_alias_2() {
  int local;
  int *local_alias = &local;
  local = 1000;
  TestData[*local_alias] = 0; // BUG
}

struct ABC {
  int a;
  int b[10];
  int c;
};

void struct_member_init() {
  struct ABC abc = {1000,{0},3};
  TestData[abc.a] = 0; // BUG
}

void struct_member_assign(struct ABC *abc) {
  abc->a = 1000;
  TestData[abc->a] = 0; // BUG
}

void struct_arraymember(struct ABC *abc) {
  abc->b[3] = 1000;
  TestData[abc->b[3]] = 0; // BUG
}
