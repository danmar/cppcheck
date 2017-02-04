
//////////////////////////////
// control flow analysis
//////////////////////////////

int buf[2];

void in_if(int a) {
  if (a==100)
    buf[a] = 0; // BUG
}

void before_if(int a) {
  buf[a] = 0; // WARNING
  if (a==100) {}
}

void after_if(int a) {
  if (a==100) {}
  buf[a] = 0; // WARNING
}

void in_for(void) {
  int x;
  for (x = 0; x<100; x++) {
    buf[x] = 0; // BUG
  }
}

void after_for(void) {
  int x;
  for (x = 0; x<100; x++) {}
  buf[x] = 0; // BUG
}

void in_switch(int x) {
  switch (x) {
  case 100:
    buf[x] = 0; // BUG
    break;
  }
}

void before_switch(int x) {
  buf[x] = 0; // WARNING
  switch (x) {
  case 100:
    break;
  }
}

void after_switch(int x) {
  switch (x) {
  case 100:
    break;
  }
  buf[x] = 0; // WARNING
}

void in_while(void) {
  int x = 0;
  while (x<100) {
    buf[x] = 0; // BUG
    x++;
  }
}

void after_while(void) {
  int x = 0;
  while (x<100)
    x++;
  buf[x] = 0; // BUG
}
