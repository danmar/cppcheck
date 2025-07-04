
# suspiciousComma

**Message**: There is a suspicious comma expression directly after a function call in an if/while condition statement, is there a misplaced paranthesis?<br/>
**Category**: ImproveCheck<br/>
**Severity**: Style<br/>
**Language**: C and C++

## Description

When calling functions in an if or while condition statement, if the conditon is complex containing many parentheses, sometimes the parenthese might be misplaced if not carefull enough. If the function has a default argument, the compiler will not report errors, and the error may not be found and cause some unexpected result while the program running.
```
int func(int a, int b = 100) {
    return a+b;
}

int call(int var) {
    if(func(func(100, 100)), var) {} //what actually wanted might be func(func(100, 100), var).
}
```
This is just to detect a most likely error, which could be some code wrote as intended with a bad style but not a real coding error.

## How to fix

The fix is simple. Just check the code carefully and make sure the function calls are as intended.
