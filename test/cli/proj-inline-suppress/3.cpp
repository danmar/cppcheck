// From forum: https://sourceforge.net/p/cppcheck/discussion/general/thread/e67653efdb/
void foo()
{ // cppcheck-suppress zerodiv
    int x = 10000 / 0;
}

