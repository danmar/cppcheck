
# cstyleCast

**Message**: C-style pointer casting<br/>
**Category**: Type Safety<br/>
**Severity**: Style<br/>
**Language**: C++, not applicable for C code

## Motivation

C style casts can be dangerous in many ways:
 * loss of precision
 * loss of sign
 * loss of constness
 * invalid type conversion

## Philosophy

This checker warns about old style C casts that can be dangerous.

This checker is not about readability. It is about safety.

This checker focus on invalid type conversions:
If there is loss of sign or precision in the cast; it's hard to know for Cppcheck if that is intentional.
If there is loss of constness; there are other checkers available and these apply to C code also.
Therefore we only warn about pointer/reference casts that could be invalid type conversions.

## Other tools / checkers

Extended checking; if you want more warnings that warn about all C style casts:
 * Misra C++ 8.2.2, cover all C style casts.
 * C style cast compiler warnings (i.e. clang reports -Wold-style-cast)

Complementing; checking for loss of constness etc:
 * Misra C 11.8
 * Loss of constness compiler warnings (i.e. clang reports -Wcast-qual).

## Examples

### Compliant C style cast

The goal is to not warn about "safe" and/or "intentional" casts.

For example:
```cpp
int *p = (int*)0;
```
A static_cast can be used however technically it's the same as the C style cast.

### Non compliant C style cast

A cast from a base class pointer to a derived class pointer is potentially unsafe:
```cpp
struct Base{};
struct Derived: public Base {};
void foo(Base* base) {
    Derived *p = (Derived*)base; // <- dangerous
}
```

## How to fix

You can use `dynamic_cast` or `static_cast` to fix warnings.

Before:
```cpp
struct Base{};
struct Derived: public Base {};
void foo(Base* base) {
    Derived *p = (Derived*)base; // <- dangerous
}
```

After:
```cpp
struct Base{};
struct Derived: public Base {};
void foo(Base* base) {
    Derived *p = dynamic_cast<Derived*>(base);
}
```
