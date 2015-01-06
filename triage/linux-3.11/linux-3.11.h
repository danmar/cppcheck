
#define __alignof__(x)            1
#define offsetof(TYPE, MEMBER)    ((size_t) &((TYPE *)0)->MEMBER)
#define ARRAY_SIZE(A)             (sizeof(A) / sizeof(A[0]))
#define BUG_ON(C)                 if (C) exit(1)
#define GOTO(label, rc)           goto label
#define RETURN(x)                 return x
#define NVERSION(version)         (version >> 16) & 0xFF, (version >> 8) & 0xFF, version & 0xFF
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

#define __get_user(x, ptr)   ((x) = somevalue + sizeof(*ptr))

void panic(const char *fmt, ...) __attribute__((noreturn));


