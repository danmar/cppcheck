oid foo(void) {
    struct s { struct s *s; }
    union { struct s s; } u = {0};
}
