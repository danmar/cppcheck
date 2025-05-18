
# cstyleCast

**Category**: Type Safety<br/>
**Severity**: Style<br/>
**Language**: C++, not applicable for C code

## Motivation

Casts can be dangerous:
 * there can be unintentional loss of precision
 * there can be unintentional loss of sign
 * there can be unintentional loss of constness
 * there can be invalid type conversions

## Philosophy

This checker warns about old style C casts.

This checker is not about readability. It is about safety.

We only want to warn if there are possible safety issues and a C++ cast can provide better safety.

## Other tools / complements

Related checkers:
 * Misra C 11.8 is violated if there is "loss of constness"
 * Misra C++ 8.2.2 is more pedantic than this checker and warns about all C style casts.
 * Warnings about "loss of constness" may be written by compilers (gcc/clang reports Wcast-qual).

## Example

### Compliant C style cast

The goal is to not warn about "safe" and/or "intentional" casts.

For example:
```
int *p = (int*)0;
```

### Non compliant C style cast

A cast from a base class pointer to a derived class pointer is potentially unsafe:
```
Derived *p = (Derived*)base;
```
If it is obvious that `base` always point at a Derived object the cast is safe and ideally no warning should be reported.
But otherwise a `dynamic_cast` is safer.

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

