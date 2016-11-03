
//////////////////////////////
// path sensitive analysis
//////////////////////////////

int buf[2];
int getValue(void); // unknown int value

// arg
// -------------------------------------

void arg_in_if(int a) {
  if (a>=100)
    buf[a] = 0; // BUG
}

void arg_before_if(int a) {
  buf[a] = 0; // WARNING
  if (a==100) {}
}

void arg_after_if(int a) {
  if (a==100) {}
  buf[a] = 0; // WARNING
}

// var
// -------------------------------------

void var(void) {
  int x = 100;
  buf[x] = 0; // BUG
}

void var_in_for(void) {
  int x;
  for (x = 0; x<100; x++) {
    buf[x] = 0; // BUG
  }
}

void var_after_for(void) {
  int x;
  for (x = 0; x<100; x++) {}
  buf[x] = 0; // BUG
}

void var_in_switch(void) {
  int x = getValue();
  switch (x) {
  case 100:
    buf[x] = 0; // BUG
    break;
  }
}

void var_before_switch(void) {
  int x = getValue();
  buf[x] = 0; // WARNING
  switch (x) {
  case 100:
    break;
  }
}

void var_after_switch(void) {
  int x = getValue();
  switch (x) {
  case 100:
    break;
  }
  buf[x] = 0; // WARNING
}

void var_in_while(void) {
  int x = 0;
  while (x<100) {
    buf[x] = 0; // BUG
	x++;
  }
}

void var_after_while(void) {
  int x = 0;
  while (x<100)
    x++;
  buf[x] = 0; // BUG
}

// arg+var
// -------------------------------------

void arg_var_assign_in_if(int a) {
  int x = 0;
  if (a==0)
    x = 100;
  buf[x] = 0; // WARNING
}

void arg_var_in_while_1(int a) {
  int x = 0;
  if (a == 100) {}
  while (x<a) {
    buf[x] = 0; // WARNING
	x++;
  }
}

void arg_var_in_while_2(int a) {
  int x = 0;
  while (x<a) {
    buf[x] = 0; // WARNING
	x++;
  }
  if (a == 100) {}
}

void arg_var_after_while(int a) {
  int x = 0;
  if (a == 100) {}
  while (x<a)
    x++;
  buf[x] = 0; // WARNING
}
