
//////////////////////////////
// path sensitive analysis
//////////////////////////////

int buf[2];
int getValue(void); // unknown int value

// arg
// -------------------------------------

void bug_arg_in_if(int a) {
  if (a>=__LINE__)
    buf[a] = 0; // BUG
}

void bug_arg_before_if(int a) {
  buf[a] = 0; // BUG
  if (a==__LINE__) {}
}

void bug_arg_after_if(int a) {
  if (a==__LINE__) {}
  buf[a] = 0; // BUG
}

// var
// -------------------------------------

void bug_var(void) {
  int x = __LINE__;
  buf[x] = 0; // BUG
}

void bug_var_in_switch(void) {
  int x = getValue();
  switch (x) {
  case __LINE__:
    buf[x] = 0; // BUG
    break;
  }
}

void bug_var_after_switch(void) {
  int x = getValue();
  switch (x) {
  case __LINE__:
    break;
  }
  buf[x] = 0; // BUG
}

void bug_var_after_while(void) {
  int x = 0;
  while (x<=__LINE__)
    x++;
  buf[x] = 0; // BUG
}

void bug_var_in_while(void) {
  int x = 0;
  while (x<__LINE__) {
    buf[x] = 0; // BUG
	x++;
  }
}

// arg+var
// -------------------------------------

void bug_arg_var_assign_in_if(int a) {
  int x = 0;
  if (a==0)
    x = __LINE__;
  buf[x] = 0; // BUG
}

void bug_arg_var_second_if(int a) {
  int x = 0;
  if (a==0)
    x = __LINE__;
  if (a+1==1)
    buf[x] = 0; // BUG
}

void bug_arg_var_after_while(int a) {
  int x = 0;
  if (a == __LINE__) {}
  while (x<a)
    x++;
  buf[x] = 0; // BUG
}

void bug_arg_var_in_while(int a) {
  int x = 0;
  if (a == __LINE__) {}
  while (x<a) {
    buf[x] = 0; // BUG
	x++;
  }
}
