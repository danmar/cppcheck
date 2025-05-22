
# cstyleCast

**Message**: C-style pointer casting<br/>
**Category**: Modernization<br/>
**Severity**: Style<br/>
**Language**: C++, not applicable for C code

## Description

Many developers feel that it's best practice to replace C casts with C++ casts.

This checker is about C casts that converts to/from a pointer or reference.

Potential safety issues are covered by other warnings so this ID `cstyleCast` is primarily about writing warnings for casts that are safe.

## How to fix

You can use C++ casts such as `static_cast` to fix these warnings.

Before:
```cpp
struct Base{};
struct Derived: public Base {};
void foo(Base* base) {
    Base *p = (Base*)derived; // <- cstyleCast, safe cast from derived object to base object
}
```

After:
```cpp
struct Base{};
struct Derived: public Base {};
void foo(Base* base) {
    Derived *p = static_cast<Derived*>(base);
}
```
