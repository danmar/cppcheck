
// Test library configuration for posix.cfg
//
// Usage:
// $ cppcheck --check-library --library=posix --enable=information --error-exitcode=1 --inline-suppr cfg/test/posix.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <unistd.h>

void bufferAccessOutOf(int fd) {
  char a[5];
  read(fd,a,5);
  // cppcheck-suppress bufferAccessOutOfBounds
  read(fd,a,6);
  write(fd,a,5);
  // cppcheck-suppress bufferAccessOutOfBounds
  write(fd,a,6);
  recv(fd,a,5,0);
  // cppcheck-suppress bufferAccessOutOfBounds
  recv(fd,a,6,0);
  recvfrom(fd,a,5,0,0x0,0x0);
  // cppcheck-suppress bufferAccessOutOfBounds
  recvfrom(fd,a,6,0,0x0,0x0);
  send(fd,a,5,0);
  // cppcheck-suppress bufferAccessOutOfBounds
  send(fd,a,6,0);
  sendto(fd,a,5,0,0x0,0x0);
  // cppcheck-suppress bufferAccessOutOfBounds
  sendto(fd,a,6,0,0x0,0x0);
  0;
}

void f(char *p) {
    isatty (0);
    mkdir (p, 0);
    getcwd (0, 0);
    // cppcheck-suppress nullPointer
    readdir (0);
}

