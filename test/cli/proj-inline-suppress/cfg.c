void f()
{
#if VER_CHECK(3, 1, 6)
    // cppcheck-suppress id
    (void)0;
#endif

#if DEF_1
    // cppcheck-suppress id
    (void)0;
#endif

    // cppcheck-suppress id
    (void)0;
}
