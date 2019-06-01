
# Speeding up Cppcheck

It is possible to speed up Cppcheck analysis in a few different ways.

## Preprocessor configurations

Imagine this source code:

    void foo()
    {
        int x;
    #ifdef __GNUC__
        x = 0;
    #endif
    #ifdef _MSC_VER
        x = 1;
    #endif
        return x;
    }

By default Cppcheck will try to check all the configurations. There are 3 important configurations here:

- Neither `__GNUC__` nor `_MSC_VER` is defined
- `__GNUC__` is defined
- `_MSC_VER` is defined

When you run Cppcheck, the output will be something like:

    $ cppcheck test.c
    Checking test.c ...
    [test.c:10]: (error) Uninitialized variable: x
    Checking test.c: __GNUC__...
    Checking test.c: _MSC_VER...

Now if you want you can limit the analysis. You probably know what the target compiler is. If `-D` is supplied and you do not specify `--force` then Cppcheck will only check the configuration you give.

    $ cppcheck -D __GNUC__ test.c
    Checking test.c ...
    Checking test.c: __GNUC__=1...

## Unused templates

If you think Cppcheck is slow and you are using templates, then you should try how it works to remove unused templates.

Imagine this code:

    template <class T> struct Foo {
        T x = 100;
    };

    template <class T> struct Bar {
        T x = 200 / 0;
    };

    int main() {
        Foo<int> foo;
        return 0;
    }

Cppcheck says:

    $ cppcheck test.cpp
    Checking test.cpp ...
    [test.cpp:7]: (error) Division by zero.

It complains about division by zero in `Bar` even though `Bar` is not instantiated.

You can use the option `--remove-unused-templates` to remove unused templates from Cppcheck analysis.

Example:

    $ cppcheck --remove-unused-templates test.cpp
    Checking test.cpp ...

This lost message is in theory not critical, since `Bar` is not instantiated the division by zero should not occur in your real program.

The speedup you get can be remarkable.

## Check headers

TBD

