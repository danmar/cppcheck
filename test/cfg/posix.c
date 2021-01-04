
// Test library configuration for posix.cfg
//
// Usage:
// $ cppcheck --check-library --library=posix --enable=information --error-exitcode=1 --inline-suppr --suppress=missingIncludeSystem test/cfg/posix.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <stdlib.h>
#include <stdio.h> // <- FILE
#include <dirent.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <dlfcn.h>
#include <fcntl.h>
// unavailable on some linux systems #include <ndbm.h>
#include <netdb.h>
#include <regex.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <syslog.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdbool.h>

bool invalidFunctionArgBool_isascii(bool b, int c)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)isascii(b);
    // cppcheck-suppress invalidFunctionArgBool
    return isascii(c != 0);
}

void uninitvar_putenv(char * envstr)
{
    // No warning is expected
    (void)putenv(envstr);

    char * p;
    // cppcheck-suppress uninitvar
    (void)putenv(p);
}

void nullPointer_putenv(char * envstr)
{
    // No warning is expected
    (void)putenv(envstr);

    char * p=NULL;
    // cppcheck-suppress nullPointer
    (void)putenv(p);
}

void memleak_scandir(void)
{
    struct dirent **namelist;
    int n = scandir(".", &namelist, NULL, alphasort);
    if (n == -1) {
        return;
    }

    // http://man7.org/linux/man-pages/man3/scandir.3.html
    /* The scandir() function scans the directory dirp, calling filter() on
       each directory entry.  Entries for which filter() returns nonzero are
       stored in strings allocated via malloc(3), sorted using qsort(3) with
       the comparison function compar(), and collected in array namelist
       which is allocated via malloc(3).  If filter is NULL, all entries are
       selected.*/

    // TODO: cppcheck-suppress memleak
}

void no_memleak_scandir(void)
{
    struct dirent **namelist;
    int n = scandir(".", &namelist, NULL, alphasort);
    if (n == -1) {
        return;
    }
    while (n--) {
        free(namelist[n]);
    }
    free(namelist);
}

void validCode(va_list valist_arg1, va_list valist_arg2)
{
    void *ptr;
    if (posix_memalign(&ptr, sizeof(void *), sizeof(void *)) == 0)
        free(ptr);
    syslog(LOG_ERR, "err %u", 0U);
    syslog(LOG_WARNING, "warn %d %d", 5, 1);
    vsyslog(LOG_EMERG, "emerg %d", valist_arg1);
    vsyslog(LOG_INFO, "test %s %d %p", valist_arg2);

    void* handle = dlopen("/lib.so", RTLD_NOW);
    if (handle) {
        dlclose(handle);
    }
}

void bufferAccessOutOfBounds(int fd)
{
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
    readlink("path", a, 5);
    // cppcheck-suppress bufferAccessOutOfBounds
    readlink("path", a, 6);
    readlinkat(1, "path", a, 5);
    // cppcheck-suppress bufferAccessOutOfBounds
    readlinkat(1, "path", a, 6);
    // This is valid
    gethostname(a, 5);
    // cppcheck-suppress bufferAccessOutOfBounds
    gethostname(a, 6);
}

void nullPointer(char *p, int fd, pthread_mutex_t mutex)
{
    // cppcheck-suppress ignoredReturnValue
    isatty(0);
    mkdir(p, 0);
    getcwd(0, 0);
    // cppcheck-suppress nullPointer
    // cppcheck-suppress readdirCalled
    readdir(0);
    // cppcheck-suppress nullPointer
    // cppcheck-suppress utimeCalled
    utime(NULL, NULL);
    // not implemented yet: cppcheck-suppress nullPointer
    read(fd,NULL,42);
    read(fd,NULL,0);
    // not implemented yet: cppcheck-suppress nullPointer
    write(fd,NULL,42);
    write(fd,NULL,0);
    // cppcheck-suppress leakReturnValNotUsed
    // cppcheck-suppress nullPointer
    open(NULL, 0);
    // cppcheck-suppress leakReturnValNotUsed
    // cppcheck-suppress nullPointer
    open(NULL, 0, 0);
    // cppcheck-suppress unreadVariable
    // cppcheck-suppress nullPointer
    int ret = access(NULL, 0);
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress leakReturnValNotUsed
    // cppcheck-suppress nullPointer
    fdopen(fd, NULL);
    // cppcheck-suppress strtokCalled
    // cppcheck-suppress nullPointer
    strtok(p, NULL);

    // cppcheck-suppress nullPointer
    pthread_mutex_init(NULL, NULL);
    // Second argument can be NULL
    pthread_mutex_init(&mutex, NULL);
    // cppcheck-suppress nullPointer
    pthread_mutex_destroy(NULL);
    // cppcheck-suppress nullPointer
    pthread_mutex_lock(NULL);
    // cppcheck-suppress nullPointer
    (void)pthread_mutex_trylock(NULL);
    // cppcheck-suppress nullPointer
    pthread_mutex_unlock(NULL);
}

void memleak_getaddrinfo()
{
    //TODO: nothing to report yet, see http://sourceforge.net/p/cppcheck/discussion/general/thread/d9737d5d/
    struct addrinfo * res=NULL;
    getaddrinfo("node", NULL, NULL, &res);
    freeaddrinfo(res);
}

void memleak_mmap(int fd)
{
    // cppcheck-suppress unreadVariable
    void *addr = mmap(NULL, 255, PROT_NONE, MAP_PRIVATE, fd, 0);
    // cppcheck-suppress memleak
}

void * memleak_mmap2() // #8327
{
    void * data = mmap(NULL, 10, PROT_READ, MAP_PRIVATE, 1, 0);
    if (data != MAP_FAILED)
        return data;
    return NULL;
}

void resourceLeak_fdopen(int fd)
{
    // cppcheck-suppress unreadVariable
    FILE *f = fdopen(fd, "r");
    // cppcheck-suppress resourceLeak
}

void resourceLeak_mkstemp(char *template)
{
    // cppcheck-suppress unreadVariable
    int fp = mkstemp(template);
    // cppcheck-suppress resourceLeak
}

void no_resourceLeak_mkstemp_01(char *template)
{
    int fp = mkstemp(template);
    close(fp);
}

int no_resourceLeak_mkstemp_02(char *template)
{
    return mkstemp(template);
}

void resourceLeak_fdopendir(int fd)
{
    // cppcheck-suppress unreadVariable
    DIR* leak1 = fdopendir(fd);
    // cppcheck-suppress resourceLeak
}

void resourceLeak_opendir(void)
{
    // cppcheck-suppress unreadVariable
    DIR* leak1 = opendir("abc");
    // cppcheck-suppress resourceLeak
}

void resourceLeak_socket(void)
{
    // cppcheck-suppress unreadVariable
    int s = socket(AF_INET, SOCK_STREAM, 0);
    // cppcheck-suppress resourceLeak
}

void resourceLeak_open1(void)
{
    // cppcheck-suppress unreadVariable
    int fd = open("file", O_RDWR | O_CREAT);
    // cppcheck-suppress resourceLeak
}

void resourceLeak_open2(void)
{
    // cppcheck-suppress unreadVariable
    int fd = open("file", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    // cppcheck-suppress resourceLeak
}

void noleak(int x, int y, int z)
{
    DIR *p1 = fdopendir(x);
    closedir(p1);
    DIR *p2 = opendir("abc");
    closedir(p2);
    int s = socket(AF_INET,SOCK_STREAM,0);
    close(s);
    int fd1 = open("a", O_RDWR | O_CREAT);
    close(fd1);
    int fd2 = open("a", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    close(fd2);
    /* TODO: add configuration for open/fdopen
        // #2830
        int fd = open("path", O_RDONLY);
        FILE *f = fdopen(fd, "rt");
        fclose(f);
    */
}


// unused return value

void ignoredReturnValue(void *addr, int fd)
{
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress leakReturnValNotUsed
    mmap(addr, 255, PROT_NONE, MAP_PRIVATE, fd, 0);
    // cppcheck-suppress ignoredReturnValue
    getuid();
    // cppcheck-suppress ignoredReturnValue
    access("filename", 1);
    // no ignoredReturnValue shall be shown for
    setuid(42);
}


// valid range

void invalidFunctionArg()
{
    // cppcheck-suppress invalidFunctionArg
    // cppcheck-suppress usleepCalled
    usleep(-1);
    // cppcheck-suppress usleepCalled
    usleep(0);
    // cppcheck-suppress usleepCalled
    usleep(999999);
    // cppcheck-suppress invalidFunctionArg
    // cppcheck-suppress usleepCalled
    usleep(1000000);
}

void invalidFunctionArg_close(int fd)
{
    if (fd < 0) {
        // cppcheck-suppress invalidFunctionArg
        (void)close(fd);
    }
}

void uninitvar(int fd)
{
    int x1, x2, x3, x4;
    char buf[2];
    int decimal, sign;
    double d;
    void *p;
    pthread_mutex_t mutex, mutex1, mutex2, mutex3;
    // cppcheck-suppress uninitvar
    write(x1,"ab",2);
    // TODO cppcheck-suppress uninitvar
    write(fd,buf,2); // #6325
    // cppcheck-suppress uninitvar
    write(fd,"ab",x2);
    // cppcheck-suppress uninitvar
    write(fd,p,2);


    /* int regcomp(regex_t *restrict preg, const char *restrict pattern, int cflags); */
    regex_t reg;
    const char * pattern;
    int cflags1, cflags2;
    // cppcheck-suppress uninitvar
    regcomp(&reg, pattern, cflags1);
    pattern="";
    // cppcheck-suppress uninitvar
    regcomp(&reg, pattern, cflags2);
    regerror(0, &reg, 0, 0);
#ifndef __CYGWIN__
    // cppcheck-suppress uninitvar
    // cppcheck-suppress unreadVariable
    // cppcheck-suppress ecvtCalled
    char *buffer = ecvt(d, 11, &decimal, &sign);
#endif
    // cppcheck-suppress gcvtCalled
    gcvt(3.141, 2, buf);

    char *filename;
    struct utimbuf *times;
    // cppcheck-suppress uninitvar
    // cppcheck-suppress utimeCalled
    utime(filename, times);
    struct timeval times1[2];
    // cppcheck-suppress uninitvar
    // cppcheck-suppress utimeCalled
    utime(filename, times1);

    // cppcheck-suppress unreadVariable
    // cppcheck-suppress uninitvar
    int access_ret = access("file", x3);

    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress leakReturnValNotUsed
    // cppcheck-suppress uninitvar
    fdopen(x4, "rw");

    char *strtok_arg1;
    // cppcheck-suppress strtokCalled
    // cppcheck-suppress uninitvar
    strtok(strtok_arg1, ";");

    // cppcheck-suppress uninitvar
    pthread_mutex_lock(&mutex1);
    // cppcheck-suppress uninitvar
    (void)pthread_mutex_trylock(&mutex2);
    // cppcheck-suppress uninitvar
    pthread_mutex_unlock(&mutex3);
    // after initialization it must be OK to call lock, trylock and unlock for this mutex
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_lock(&mutex);
    (void)pthread_mutex_trylock(&mutex);
    pthread_mutex_unlock(&mutex);
}

void uninitvar_getcwd(void)
{
    char *buf;
    size_t size;
    // cppcheck-suppress uninitvar
    (void)getcwd(buf,size);
}


void uninitvar_types(void)
{
    // cppcheck-suppress unassignedVariable
    blkcnt_t b;
    // cppcheck-suppress uninitvar
    b + 1;

    struct dirent d;
    // TODO cppcheck-suppress uninitvar
    d.d_ino + 1;
}

void timet_h(struct timespec* ptp1)
{
    clockid_t clk_id1, clk_id2, clk_id3;
    struct timespec* ptp;
    // cppcheck-suppress uninitvar
    clock_settime(CLOCK_REALTIME, ptp);
    // cppcheck-suppress uninitvar
    clock_settime(clk_id1, ptp);
    // cppcheck-suppress uninitvar
    clock_settime(clk_id2, ptp1);

    struct timespec tp;
    // TODO cppcheck-suppress uninitvar
    clock_settime(CLOCK_REALTIME, &tp); // #6577 - false negative
    // cppcheck-suppress uninitvar
    clock_settime(clk_id3, &tp);

    time_t clock = time(0);
    char buf[26];
    // cppcheck-suppress ctime_rCalled
    ctime_r(&clock, buf);
}

void dl(const char* libname, const char* func)
{
    void* lib = dlopen(libname, RTLD_NOW);
    // cppcheck-suppress redundantInitialization
    // cppcheck-suppress resourceLeak
    lib = dlopen(libname, RTLD_LAZY);
    const char* funcname;
    // cppcheck-suppress uninitvar
    // cppcheck-suppress unreadVariable
    void* sym = dlsym(lib, funcname);
    // cppcheck-suppress ignoredReturnValue
    dlsym(lib, "foo");
    void* uninit;
    // cppcheck-suppress uninitvar
    dlclose(uninit);
    // cppcheck-suppress resourceLeak
}

void asctime_r_test(struct tm * tm, char * bufSizeUnknown)
{
    struct tm tm_uninit_data;
    struct tm * tm_uninit_pointer;
    char bufSize5[5];
    char bufSize25[25];
    char bufSize26[26];
    char bufSize100[100];

    // cppcheck-suppress asctime_rCalled
    // cppcheck-suppress bufferAccessOutOfBounds
    asctime_r(tm, bufSize5);
    // cppcheck-suppress asctime_rCalled
    // cppcheck-suppress bufferAccessOutOfBounds
    asctime_r(tm, bufSize25);
    // cppcheck-suppress asctime_rCalled
    asctime_r(tm, bufSize26);
    // cppcheck-suppress asctime_rCalled
    asctime_r(tm, bufSize100);

    // cppcheck-suppress asctime_rCalled
    // cppcheck-suppress uninitvar
    asctime_r(&tm_uninit_data, bufSize100);
    // cppcheck-suppress asctime_rCalled
    // cppcheck-suppress uninitvar
    asctime_r(tm_uninit_pointer, bufSize100);

    // cppcheck-suppress asctime_rCalled
    asctime_r(tm, bufSizeUnknown);
}

void ctime_r_test(time_t * timep, char * bufSizeUnknown)
{
    time_t time_t_uninit_data;
    time_t * time_t_uninit_pointer;
    char bufSize5[5];
    char bufSize25[25];
    char bufSize26[26];
    char bufSize100[100];

    // cppcheck-suppress ctime_rCalled
    // cppcheck-suppress bufferAccessOutOfBounds
    ctime_r(timep, bufSize5);
    // cppcheck-suppress ctime_rCalled
    // cppcheck-suppress bufferAccessOutOfBounds
    ctime_r(timep, bufSize25);
    // cppcheck-suppress ctime_rCalled
    ctime_r(timep, bufSize26);
    // cppcheck-suppress ctime_rCalled
    ctime_r(timep, bufSize100);

    // cppcheck-suppress ctime_rCalled
    // cppcheck-suppress uninitvar
    ctime_r(&time_t_uninit_data, bufSize100);
    // cppcheck-suppress ctime_rCalled
    // cppcheck-suppress uninitvar
    ctime_r(time_t_uninit_pointer, bufSize100);

    // cppcheck-suppress ctime_rCalled
    ctime_r(timep, bufSizeUnknown);
}
