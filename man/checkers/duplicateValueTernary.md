# duplicateValueTernary

**Message**: Same value in both branches of ternary operator<br/>
**Category**: Logic<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

This checker detects when both branches of a ternary operator (`condition ? true_expr : false_expr`) evaluate to the same value, even though the expressions themselves may be syntactically different. This usually indicates a logic error where the condition serves no purpose since the result is always the same.

The key difference from `duplicateExpressionTernary` is:
- `duplicateValueTernary`: Triggered when expressions have the same **value** (e.g., `1` and `(int)1`)
- `duplicateExpressionTernary`: Triggered when expressions are syntactically **identical** (e.g., `x` and `x`)

The warning is triggered when:
- The second and third operands of a ternary operator evaluate to the same constant value
- The expressions are constant statements that don't change during evaluation
- The expressions have different syntax but equivalent values

However, no warning is generated when:
- The expressions have different values on different platforms
- Variables are modified between evaluations
- The expressions involve non-constant computations

## Examples

### Problematic code

```cpp
// Different expressions, same value
int result = condition ? (int)1 : 1;  // Warning: duplicateValueTernary

// Different cast syntax, same value
int result = condition ? 1 : (int)1;  // Warning: duplicateValueTernary

// Same constant value with different representations
int result = condition ? (int)1 : 1;  // Warning: duplicateValueTernary
```

### Code that correctly triggers duplicateExpressionTernary instead

```cpp
// Identical expressions (not just values)
int result = condition ? 1 : 1;       // Warning: duplicateExpressionTernary
int result = condition ? (int)1 : (int)1;  // Warning: duplicateExpressionTernary
```

### Fixed code

```cpp
// Different values in branches
int result = condition ? 1 : 2;  // OK

// Simplified - condition doesn't matter
int result = 1;  // OK - removed unnecessary ternary

// Platform-dependent values are allowed
int size = is_64bit ? sizeof(long) : sizeof(int);  // OK - may differ on platforms
```

### Exception: Platform-dependent values

```cpp
// This may NOT generate a warning if values differ across platforms
int size = condition ? sizeof(long) : sizeof(int);  // OK on some platforms

// Special case: +0.0 vs -0.0 are considered different
double result = condition ? 0.0 : -0.0;  // OK - different floating-point values
```

## How to fix

1. **Check for logic errors**: Verify if both branches should actually have the same value
2. **Simplify the code**: If both branches always have the same value, remove the ternary operator
3. **Use different values**: If the branches should differ, update one of them
4. **Remove unnecessary casts**: If the difference is only in casting, consider if the cast is needed

Before:
```cpp
int getValue(bool flag) {
    return flag ? (int)42 : 42;  // Same value, different expressions
}
```

After (simplified):
```cpp
int getValue(bool flag) {
    return 42;  // Simplified - condition doesn't affect result
}
```

Or (if branches should differ):
```cpp
int getValue(bool flag) {
    return flag ? 42 : 0;  // Different values for different conditions
}
```