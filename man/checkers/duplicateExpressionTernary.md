# duplicateExpressionTernary

**Message**: Same expression in both branches of ternary operator<br/>
**Category**: Logic<br/>
**Severity**: Style<br/>
**Language**: C/C++

## Description

This checker detects when syntactically identical expressions appear in both the true and false branches of a ternary operator (`condition ? true_expr : false_expr`). When this occurs, the result of the ternary operator is always the same regardless of the condition, which is usually a logic error or copy-paste mistake.

The key difference from `duplicateValueTernary` is:
- `duplicateExpressionTernary`: Triggered when expressions are syntactically **identical** (e.g., `x` and `x`, or `(int)1` and `(int)1`)
- `duplicateValueTernary`: Triggered when expressions have the same **value** but different syntax (e.g., `1` and `(int)1`)

The warning is triggered when:
- The second and third operands of a ternary operator are syntactically identical expressions
- This includes cases where variables are referenced through aliases that resolve to the same expression

However, no warning is generated when:
- The expressions have different values on different platforms (e.g., `sizeof(uint32_t)` vs `sizeof(unsigned int)` which may differ across platforms)
- The expressions involve platform-dependent behavior

## Examples

### Problematic code

```cpp
// Same expression in both branches
int result = condition ? x : x;  // Warning: duplicateExpressionTernary

// Same variable referenced through alias
const int c = a;
int result = condition ? a : c;  // Warning: duplicateExpressionTernary

// Identical expressions with same syntax
int result = condition ? 1 : 1;  // Warning: duplicateExpressionTernary
int result = condition ? (int)1 : (int)1;  // Warning: duplicateExpressionTernary
```

### Code that triggers duplicateValueTernary instead

```cpp
// Different syntax, same value (triggers duplicateValueTernary, not duplicateExpressionTernary)
int result = condition ? 1 : (int)1;  // Warning: duplicateValueTernary
int result = condition ? (int)42 : 42;  // Warning: duplicateValueTernary
```

### Fixed code

```cpp
// Different expressions in branches
int result = condition ? x : y;  // OK

// Platform-dependent sizes are allowed
int size = is_32bit ? sizeof(uint32_t) : sizeof(unsigned int);  // OK - may differ on platforms
```

### Exception: Platform-dependent expressions

```cpp
// This does NOT generate a warning because sizeof may differ across platforms
typedef float _Complex complex_f32;
typedef struct {
    uint16_t real;
    uint16_t imag;
} packed_complex;

int size = condition ? sizeof(packed_complex) : sizeof(complex_f32);  // OK
```

## How to fix

1. **Check for copy-paste errors**: Verify that both branches are supposed to have the same expression
2. **Use different expressions**: If the branches should differ, update one of them
3. **Simplify the code**: If both branches are intentionally the same, remove the ternary operator entirely

Before:
```cpp
int getValue(bool flag) {
    return flag ? computeValue() : computeValue();  // Same computation in both branches
}
```

After:
```cpp
int getValue(bool flag) {
    return computeValue();  // Simplified - condition doesn't matter
}
```

Or if the branches should differ:
```cpp
int getValue(bool flag) {
    return flag ? computeValue() : computeAlternativeValue();  // Different computations
}
```