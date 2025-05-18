
# cstyleCast

**Message**: C-style pointer casting<br/>
**Category**: Type Safety<br/>
**Severity**: Style<br/>
**Language**: C++, not applicable for C code

## Motivation

C style casts can be dangerous in many ways:
 * there can be unintentional loss of precision
 * there can be unintentional loss of sign
 * there can be unintentional loss of constness
 * there can be invalid type conversions

## Philosophy

This checker warns about old style C casts that looks dangerous.

This checker is not about readability. It is about safety.

We only want to warn if there are possible safety issues and a C++ cast can provide better safety.

## Other tools / checkers

Related checkers:
 * Complements: Misra C 11.8 is violated if there is "loss of constness"
 * More noisy: Misra C++ 8.2.2, cover all C style casts.
 * More noisy: C style cast compiler warnings (i.e. clang reports Wold-style-cast)
 * Complements: Loss of constness compiler warnings.

## Example

### Compliant C style cast

The goal is to not warn about "safe" and/or "intentional" casts.

For example:
```
int *p = (int*)0;
```
A static_cast can be used however technically it's the same as the C style cast.

Other example:
```
uint8_t x = (uint8_t)0x1234;
```
A static_cast can be used however technically it's the same as the C style cast.

### Non compliant C style cast

A cast from a base class pointer to a derived class pointer is potentially unsafe:
```
Derived *p = (Derived*)base;
```
A `dynamic_cast` is safer than the C-style cast here.

## How to fix

You can use `dynamic_cast` or `static_cast` to fix warnings.

Before:
```
Derived *p = (Derived*)base;
```

After:
```
Derived *p = dynamic_cast<Derived*>(base);
```

## Enable

--enable=style

