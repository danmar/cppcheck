
// Test library configuration for posix.cfg
//
// Usage:
// $ cppcheck --check-library --library=posix --enable=information --error-exitcode=1 --inline-suppr --suppress=missingIncludeSystem test/cfg/posix.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <stdio.h> // <- FILE
#include <dirent.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>


void bufferAccessOutOfBounds(int fd) {
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
  // cppcheck-suppress constStatement
  0;
}

void nullPointer(char *p) {
    isatty (0);
    mkdir (p, 0);
    getcwd (0, 0);
    // cppcheck-suppress nullPointer
    readdir (0);
}

void memleak_mmap(int fd) {
    // cppcheck-suppress unreadVariable
    void *addr = mmap(NULL, 255, PROT_NONE, MAP_PRIVATE, fd, 0);
    // cppcheck-suppress memleak
}

/* TODO: add configuration for fdopen
void resourceLeak_fdopen(int fd) {
    FILE *f = fdopen(fd, "r");
    // cppcheck-suppress resourceLeak
}
*/

void resourceLeak_fdopendir(int fd) {
    // cppcheck-suppress unreadVariable
    DIR* leak1 = fdopendir(fd);
    // cppcheck-suppress resourceLeak
}

void resourceLeak_opendir(void) {
    // cppcheck-suppress unreadVariable
    DIR* leak1 = opendir("abc");
    // cppcheck-suppress resourceLeak
}

void resourceLeak_socket(void) {
    // cppcheck-suppress unreadVariable
    int s = socket(AF_INET, SOCK_STREAM, 0);
    // cppcheck-suppress resourceLeak
}

void noleak(int x, int y, int z) {
    DIR *p1 = fdopendir(x); closedir(p1);
    DIR *p2 = opendir("abc"); closedir(p2);
    int s = socket(AF_INET,SOCK_STREAM,0); close(s);
/* TODO: add configuration for open/fdopen
    // #2830
    int fd = open("path", O_RDONLY);
    FILE *f = fdopen(fd, "rt");
    fclose(f);
*/
}


// unused return value

void ignoredReturnValue(void *addr, int fd) {
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress leakReturnValNotUsed
    mmap(addr, 255, PROT_NONE, MAP_PRIVATE, fd, 0);
    // cppcheck-suppress ignoredReturnValue
    setuid(42);
    // cppcheck-suppress ignoredReturnValue
    getuid();
}


// valid range

void invalidFunctionArg() {
    // cppcheck-suppress invalidFunctionArg
    usleep(-1);
    usleep(0);
    usleep(999999);
    // cppcheck-suppress invalidFunctionArg
    usleep(1000000);
}

void uninitvar(int fd) {
    int x;
    char buf[2];
    // cppcheck-suppress uninitvar
    write(x,"ab",2);
    // cppcheck-suppress uninitvar
    write(fd,buf,2);
    // cppcheck-suppress uninitvar
    write(fd,"ab",x);
}

void uninitvar_types(void) {
    // cppcheck-suppress unassignedVariable
    blkcnt_t b;
    // cppcheck-suppress uninitvar
    b + 1;

    struct dirent d;
    // cppcheck-suppress uninitvar
    d.d_ino + 1;
}
