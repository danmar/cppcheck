
#define BUG_ON(C)      if (C) exit(1)
#define ARRAY_SIZE(A)  (sizeof(A) / sizeof(A[0]))

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

void panic(const char *fmt, ...) __attribute__((noreturn));

