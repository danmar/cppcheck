
// Test library configuration for mfc.cfg

#include <afxwin.h>


class MyClass : public CObject {
    DECLARE_DYNAMIC(MyClass)
    DECLARE_DYNCREATE(MyClass)
    DECLARE_SERIAL(MyClass)
public:
    MyClass() {}
};
IMPLEMENT_DYNAMIC(MyClass, CObject)
IMPLEMENT_DYNCREATE(MyClass, CObject)
IMPLEMENT_SERIAL(MyClass,CObject, 42)