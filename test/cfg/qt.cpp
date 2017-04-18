
// Test library configuration for qt.cfg
//
// Usage:
// $ cppcheck --check-library --enable=information --error-exitcode=1 --suppress=missingIncludeSystem --inline-suppr --library=qt test/cfg/qt.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

class QString {
public:
  int size();
  char &operator[](int pos);
};

void QString1(QString s) {
  for (int i = 0; i <= s.size(); ++i) {
    // cppcheck-suppress stlOutOfBounds
    s[i] = 'x';
  }
}
