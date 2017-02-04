~/llvm/build/bin/clang -cc1 -analyze -analyzer-checker=alpha.security controlflow.c data.c functions.c 2>&1 /dev/null | grep warning
~/llvm/build/bin/clang -cc1 -analyze -analyzer-checker=alpha.security,core ub.c 2>&1 /dev/null | grep warning

