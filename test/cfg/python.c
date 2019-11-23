
// Test library configuration for python.cfg
//
// Usage:
// $ cppcheck --check-library --library=python --enable=information --error-exitcode=1 --inline-suppr --suppress=missingIncludeSystem test/cfg/python.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#define PY_SSIZE_T_CLEAN
#include <Python.h> // should be the first include
#include <stdio.h>

void validCode(PyObject * pPyObjArg)
{
    PyObject * pPyObjNULL = NULL;
    Py_Initialize();
    Py_INCREF(pPyObjArg);
    Py_DECREF(pPyObjArg);
    Py_XINCREF(pPyObjArg);
    Py_XINCREF(pPyObjNULL);
    Py_XDECREF(pPyObjArg);
    Py_XDECREF(pPyObjNULL);
    Py_CLEAR(pPyObjArg);
    Py_CLEAR(pPyObjNULL);
    (void)PyErr_NewException("text", NULL, NULL);

    char * pBuf1 = PyMem_Malloc(5);
    PyMem_Free(pBuf1);
    int * pIntBuf1 = PyMem_New(int, 10);
    PyMem_Free(pIntBuf1);
}

void nullPointer()
{
    // cppcheck-suppress nullPointer
    Py_INCREF(NULL);
    // cppcheck-suppress nullPointer
    Py_DECREF(NULL);
}

void PyMem_Malloc_memleak()
{
    char * pBuf1 = PyMem_Malloc(1);
    printf("%p", pBuf1);
    // cppcheck-suppress memleak
}

void PyMem_Malloc_mismatchAllocDealloc()
{
    char * pBuf1 = PyMem_Malloc(10);
    // cppcheck-suppress mismatchAllocDealloc
    free(pBuf1);
}

void PyMem_New_memleak()
{
    char * pBuf1 = PyMem_New(char, 5);
    printf("%p", pBuf1);
    // cppcheck-suppress memleak
}
