
int TestData[10];

int g;
void global() {
  g = 1000;
  TestData[g] = 0; // BUG
}

int garr[10];
void global_array() {
  garr[3] = 1000;
  TestData[garr[3]] = 0; // BUG
}

int *gp;
void global_pointer() {
  *gp = 1000;
  TestData[*gp] = 0; // BUG
}


void local() {
  int x;
  x = 1000;
  TestData[x] = 0; // BUG
}

void local_array() {
  int arr[10];
  arr[3] = 1000;
  TestData[arr[3]] = 0; // BUG
}

void local_alias_1() {
  int x;
  int *p = &x;
  *p = 1000;
  TestData[*p] = 0; // BUG
}

void local_alias_2() {
  int x;
  int *p = &x;
  x = 1000;
  TestData[*p] = 0; // BUG
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
