/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tokenize.h"
#include "tokenlist.h"
#include "checkmemoryleak.h"
#include "testsuite.h"
#include "symboldatabase.h"
#include "preprocessor.h"


class TestMemleak : private TestFixture {
public:
    TestMemleak() : TestFixture("TestMemleak") {
    }

private:
    void run() {
        TEST_CASE(testFunctionReturnType);
        TEST_CASE(open);
    }

    CheckMemoryLeak::AllocType functionReturnType(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        CheckMemoryLeak c(&tokenizer, this, &settings);

        return c.functionReturnType(&tokenizer.getSymbolDatabase()->scopeList.front().functionList.front());
    }

    void testFunctionReturnType() {
        {
            const char code[] = "const char *foo()\n"
                                "{ return 0; }";
            ASSERT_EQUALS(CheckMemoryLeak::No, functionReturnType(code));
        }

        {
            const char code[] = "Fred *newFred()\n"
                                "{ return new Fred; }";
            ASSERT_EQUALS(CheckMemoryLeak::New, functionReturnType(code));
        }

        {
            const char code[] = "char *foo()\n"
                                "{ return new char[100]; }";
            ASSERT_EQUALS(CheckMemoryLeak::NewArray, functionReturnType(code));
        }

        {
            const char code[] = "char *foo()\n"
                                "{\n"
                                "    char *p = new char[100];\n"
                                "    return p;\n"
                                "}";
            ASSERT_EQUALS(CheckMemoryLeak::NewArray, functionReturnType(code));
        }
    }

    void open() {
        const char code[] = "class A {\n"
                            "  static int open() {\n"
                            "    return 1;\n"
                            "  }\n"
                            "\n"
                            "  A() {\n"
                            "    int ret = open();\n"
                            "  }\n"
                            "};\n";

        // Clear the error buffer..
        errout.str("");

        Settings settings;

        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // there is no allocation
        const Token *tok = Token::findsimplematch(tokenizer.tokens(), "ret =");
        CheckMemoryLeak check(&tokenizer, 0, &settings);
        ASSERT_EQUALS(CheckMemoryLeak::No, check.getAllocationType(tok->tokAt(2), 1));
    }
};

static TestMemleak testMemleak;





class TestMemleakInFunction : public TestFixture {
public:
    TestMemleakInFunction() : TestFixture("TestMemleakInFunction") {
    }

private:
    Settings settings1;
    Settings settings2;

    void check(const char code[], const Settings *settings = nullptr, bool c = false) {
        // Clear the error buffer..
        errout.str("");

        if (!settings)
            settings = &settings1;

        // Tokenize..
        Tokenizer tokenizer(settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, c?"test.c":"test.cpp");
        tokenizer.simplifyTokenList2();

        // Check for memory leaks..
        CheckMemoryLeakInFunction checkMemoryLeak(&tokenizer, settings, this);
        checkMemoryLeak.checkReallocUsage();
        checkMemoryLeak.check();
    }


    void run() {
        LOAD_LIB_2(settings1.library, "std.cfg");
        LOAD_LIB_2(settings1.library, "gtk.cfg");
        LOAD_LIB_2(settings2.library, "std.cfg");

        // Check that getcode works correctly..
        TEST_CASE(testgetcode);

        // check that call_func works correctly..
        TEST_CASE(call_func);

        // Check that simplifycode works correctly..
        TEST_CASE(simplifycode);

        // Check that errors are found..
        TEST_CASE(findleak);

        TEST_CASE(simple5);
        TEST_CASE(simple7);
        TEST_CASE(simple9);     // Bug 2435468 - member function "free"
        TEST_CASE(simple11);
        TEST_CASE(nonstd_free);
        TEST_CASE(new_nothrow);

        TEST_CASE(staticvar);
        TEST_CASE(externvar);
        TEST_CASE(referencevar);  // 3954 - false positive for reference pointer

        TEST_CASE(alloc_alloc_1);

        TEST_CASE(ifelse6);
        TEST_CASE(ifelse7);
        TEST_CASE(ifelse8);
        TEST_CASE(ifelse10);

        TEST_CASE(if4);
        TEST_CASE(if7);     // Bug 2401436
        TEST_CASE(if8);     // Bug 2458532
        TEST_CASE(if9);     // if (realloc)
        TEST_CASE(if10);    // else if (realloc)
        TEST_CASE(if11);

        TEST_CASE(forwhile5);
        TEST_CASE(forwhile6);
        TEST_CASE(forwhile8);       // Bug 2429936
        TEST_CASE(forwhile9);
        TEST_CASE(forwhile10);
        TEST_CASE(forwhile11);

        TEST_CASE(switch2);
        TEST_CASE(switch3);

        TEST_CASE(ret5);        // Bug 2458436 - return use
        TEST_CASE(ret6);
        TEST_CASE(ret7);
        TEST_CASE(ret8);

        TEST_CASE(mismatch1);
        TEST_CASE(mismatch2);
        TEST_CASE(mismatch3);
        TEST_CASE(mismatch4);
        TEST_CASE(mismatch5);
        TEST_CASE(mismatch6);

        TEST_CASE(mismatchSize);

        TEST_CASE(func3);
        TEST_CASE(func4);
        TEST_CASE(func5);
        TEST_CASE(func6);
        TEST_CASE(func7);
        TEST_CASE(func9);       // Embedding the function call in a if-condition
        TEST_CASE(func10);      // Bug 2458510 - Function pointer
        TEST_CASE(func11);      // Bug 2458510 - Function pointer
        TEST_CASE(func12);
        TEST_CASE(func13);
        TEST_CASE(func14);
        TEST_CASE(func15);
        TEST_CASE(func16);
        TEST_CASE(func17);
        TEST_CASE(func18);
        TEST_CASE(func19);      // Ticket #2056 - if (!f(p)) return 0;
        TEST_CASE(func20);      // Ticket #2182 - exit is not handled
        TEST_CASE(func21);      // Ticket #2569
        TEST_CASE(func22);      // Ticket #2668
        TEST_CASE(func23);      // Ticket #2667
        TEST_CASE(func24);      // Ticket #2705
        TEST_CASE(func25);      // Ticket #2904
        TEST_CASE(func26);
        TEST_CASE(func27);      // Ticket #2773
        TEST_CASE(func28);      // Ticket #3236

        TEST_CASE(allocfunc1);
        TEST_CASE(allocfunc2);
        TEST_CASE(allocfunc3);
        TEST_CASE(allocfunc4);
        TEST_CASE(allocfunc5);
        TEST_CASE(allocfunc6);
        TEST_CASE(allocfunc7);
        TEST_CASE(allocfunc8);
        TEST_CASE(allocfunc9);
        TEST_CASE(allocfunc10);
        TEST_CASE(allocfunc11);
        TEST_CASE(allocfunc12); // #3660: allocating and returning non-local pointer => not allocfunc
        TEST_CASE(allocfunc13); // Ticket #4494 and #4540 - class function
        TEST_CASE(allocfunc14); // Use pointer before returning it

        TEST_CASE(throw1);
        TEST_CASE(throw2);

        TEST_CASE(linux_list_1);
        TEST_CASE(linux_list_2);

        TEST_CASE(sizeof1);

        TEST_CASE(realloc1);
        TEST_CASE(realloc2);
        TEST_CASE(realloc3);
        TEST_CASE(realloc4);
        TEST_CASE(realloc5);
        TEST_CASE(realloc6);
        TEST_CASE(realloc7);
        TEST_CASE(realloc8);
        TEST_CASE(realloc9);
        TEST_CASE(realloc10);
        TEST_CASE(realloc11);
        TEST_CASE(realloc12);
        TEST_CASE(realloc13);
        TEST_CASE(realloc14);
        TEST_CASE(realloc15);
        TEST_CASE(realloc16);

        TEST_CASE(assign1);
        TEST_CASE(assign2);   // #2806 - FP when using redundant assignment

        TEST_CASE(varid);

        TEST_CASE(cast1);

        // Using deallocated memory:
        // * It is ok to take the address to deallocated memory
        // * It is not ok to dereference a pointer to deallocated memory
        TEST_CASE(dealloc_use);
        TEST_CASE(dealloc_use_2);
        TEST_CASE(dealloc_use_3);

        // free a free'd pointer
        TEST_CASE(freefree1);
        TEST_CASE(freefree2);
        TEST_CASE(freefree3); // #4236 - FP. bar(&p)
        TEST_CASE(strcpy_result_assignment);
        TEST_CASE(strcat_result_assignment);

        TEST_CASE(all1);                // Extra checking when --all is given

        TEST_CASE(malloc_constant_1);     // Check that the malloc constant matches the type

        // Calls to unknown functions.. they may throw exception, quit the program, etc
        TEST_CASE(unknownFunction1);
        TEST_CASE(unknownFunction2);
        TEST_CASE(unknownFunction4);
        TEST_CASE(unknownFunction5);

        // detect leak in class member function..
        TEST_CASE(class1);
        TEST_CASE(class2);

        TEST_CASE(autoptr1);
        TEST_CASE(if_with_and);
        TEST_CASE(assign_pclose);

        // Using the function "exit"
        TEST_CASE(exit2);
        TEST_CASE(exit4);
        TEST_CASE(exit5);
        TEST_CASE(exit6);
        TEST_CASE(exit7);
        TEST_CASE(noreturn);

        TEST_CASE(strndup_function);
        TEST_CASE(tmpfile_function);
        TEST_CASE(fcloseall_function);
        TEST_CASE(file_functions);
        TEST_CASE(getc_function);

        TEST_CASE(open_function);
        TEST_CASE(creat_function);
        TEST_CASE(close_function);
        TEST_CASE(fd_functions);

        TEST_CASE(pointer_to_pointer);
        TEST_CASE(dealloc_and_alloc_in_func);

        // Unknown syntax
        TEST_CASE(unknownSyntax1);
        TEST_CASE(knownFunctions);

        TEST_CASE(same_function_name);

        // #1440 - Check function parameters also..
        TEST_CASE(functionParameter);

        // setjmp/longjmp..
        TEST_CASE(jmp);

        TEST_CASE(trac1949);
        TEST_CASE(trac2540);

        // #2662: segfault because of endless recursion (call_func -> getAllocationType -> functionReturnType -> call_func ..)
        TEST_CASE(trac2662);

        // #1879 non regression test case
        TEST_CASE(trac1879);

        TEST_CASE(ptrptr);

        TEST_CASE(c_code);

        TEST_CASE(gnucfg);
    }

    std::string getcode(const char code[], const char varname[], bool classfunc=false) {
        // Clear the error buffer..
        errout.str("");

        settings2.standards.posix = true;

        // Tokenize..
        Tokenizer tokenizer(&settings2, this);
        std::istringstream istr(code);
        if (!tokenizer.tokenize(istr, "test.cpp"))
            return "";
        tokenizer.simplifyTokenList2();

        const unsigned int varId(Token::findmatch(tokenizer.tokens(), varname)->varId());

        // getcode..
        CheckMemoryLeakInFunction checkMemoryLeak(&tokenizer, &settings2, nullptr);
        std::list<const Token *> callstack;
        callstack.push_back(0);
        CheckMemoryLeak::AllocType allocType, deallocType;
        allocType = deallocType = CheckMemoryLeak::No;
        Token *tokens = checkMemoryLeak.getcode(tokenizer.tokens(), callstack, varId, allocType, deallocType, classfunc, 1);

        // stringify..
        std::ostringstream ret;
        for (const Token *tok = tokens; tok; tok = tok->next())
            ret << tok->str();

        TokenList::deleteTokens(tokens);

        return ret.str();
    }




    void testgetcode() {
        // alloc;
        ASSERT_EQUALS(";;alloc;", getcode("int *a = malloc(100);", "a"));
        TODO_ASSERT_EQUALS(";;alloc;", ";;alloccallfunc;", getcode("int *a = ::malloc(100);", "a"));
        ASSERT_EQUALS(";;alloc;", getcode("int *a = new int;", "a"));
        ASSERT_EQUALS(";;alloc;", getcode("int *a = new int[10];", "a"));
        ASSERT_EQUALS(";;alloc;", getcode("int **a = new int*[10];", "a"));
        ASSERT_EQUALS(";;alloc;", getcode("int * const a = new int[10];", "a"));
        ASSERT_EQUALS(";;alloc;", getcode("const int * const a = new int[10];", "a"));
        ASSERT_EQUALS(";;alloc;", getcode("int i = open(a,b);", "i"));
        ASSERT_EQUALS(";;assign;", getcode("int i = open();", "i"));

        // alloc; return use;
        ASSERT_EQUALS(";;alloc;returnuse;", getcode("int *a = new int[10]; return a;", "a"));
        ASSERT_EQUALS(";;alloc;returnuse;", getcode("char *a = new char[100]; return (char *)a;", "a"));

        // alloc; return;
        ASSERT_EQUALS(";;alloc;return;", getcode("char *s = new char[100]; return 0;", "s"));
        ASSERT_EQUALS(";;alloc;return;", getcode("char *s = new char[100]; return s[0];", "s"));
        ASSERT_EQUALS(";;alloc;return;", getcode("char *s = new char[100]; return strcmp(s,x);", "s"));

        // lock/unlock..
        ASSERT_EQUALS(";;alloc;", getcode("int a; __cppcheck_lock();", ""));
        ASSERT_EQUALS(";;callfunc;", getcode("int a; __cppcheck_lock();", "a"));
        ASSERT_EQUALS(";;dealloc;", getcode("int a; __cppcheck_unlock();", ""));
        ASSERT_EQUALS(";;callfunc;", getcode("int a; __cppcheck_unlock();", "a"));

        // dealloc;
        ASSERT_EQUALS(";;dealloc;", getcode("char *s; free(s);", "s"));
        ASSERT_EQUALS(";;dealloc;", getcode("char *s; free((void *)s);", "s"));
        ASSERT_EQUALS(";;dealloc;", getcode("char *s; free((void *)(s));", "s"));
        ASSERT_EQUALS(";;dealloc;", getcode("char *s; free(reinterpret_cast<void *>(s));", "s"));
        ASSERT_EQUALS(";;dealloc;", getcode("char *s; ::free(s);", "s")); // #2802
        ASSERT_EQUALS(";;dealloc;", getcode("char *s; delete s;", "s"));
        ASSERT_EQUALS(";;dealloc;", getcode("char *s; delete (s);", "s"));
        TODO_ASSERT_EQUALS(";;dealloc;",
                           ";;;", getcode("char *s; delete (void *)(s);", "s"));
        ASSERT_EQUALS(";;dealloc;", getcode("char *s; delete [] s;", "s"));
        ASSERT_EQUALS(";;dealloc;", getcode("char *s; delete [] (s);", "s"));
        ASSERT_EQUALS(";;dealloc;", getcode("void *p; foo(fclose(p));", "p"));
        ASSERT_EQUALS(";;dealloc;", getcode("void *p; foo(close(p));", "p"));
        ASSERT_EQUALS(";;;;", getcode("FILE *f1; FILE *f2; fclose(f1);", "f2"));
        ASSERT_EQUALS(";;returnuse;", getcode("FILE *f; return fclose(f) == EOF ? 1 : 2;", "f"));
        ASSERT_EQUALS(";;dealloc;", getcode("char *s; s ? free(s) : 0;", "s"));

        // if..
        ASSERT_EQUALS(";;if{}", getcode("char *s; if (a) { }", "s"));
        ASSERT_EQUALS(";;dealloc;ifv{}", getcode("FILE *f; if (fclose(f)) { }", "f"));
        ASSERT_EQUALS(";;if(!var){}else{}", getcode("char *s; if (!s) { } else { }", "s"));
        TODO_ASSERT_EQUALS(";;ifv{}",";;if{}", getcode("char *s; if (a && s) { }", "s"));
        ASSERT_EQUALS(";;ifv{}", getcode("char *s; if (s && a) { }", "s"));
        ASSERT_EQUALS(";;;ifv{}", getcode("char *s; int a; if (a && s) { }", "s"));
        ASSERT_EQUALS(";;;ifv{}", getcode("char *s; int a; if (s && a) { }", "s"));
        ASSERT_EQUALS(";;ifv{}", getcode("char *s; if (a || s) { }", "s"));
        ASSERT_EQUALS(";;ifv{}", getcode("char *s; if (s || a) { }", "s"));
        ASSERT_EQUALS(";;if(!var){}", getcode("char *s; if (a && !s) { }", "s"));
        ASSERT_EQUALS(";;ifv{}", getcode("char *s; if (foo(!s)) { }", "s"));
        ASSERT_EQUALS(";;;if{dealloc;};if{dealloc;return;}assign;returnuse;", getcode("char *buf, *tmp; tmp = realloc(buf, 40); if (!(tmp)) { free(buf); return; } buf = tmp; return buf;", "buf"));
        ASSERT_EQUALS(";;if{}", getcode("FILE *f; if (fgets(buf,100,f)){}", "f"));
        ASSERT_EQUALS(";;alloc;if(var){dealloc;}", getcode("int fd = open(a,b); if (0 < fd) { close(fd); }", "fd"));
        ASSERT_EQUALS(";;use;if{}", getcode("char *s; if (x(s)) { }", "s"));
        ASSERT_EQUALS(";;use;if{}", getcode("char *s; if (x(&s)) { }", "s"));
        ASSERT_EQUALS(";;use;if{}", getcode("char *s; if (!s || x(&s)) { }", "s"));
        ASSERT_EQUALS(";;ifv{}", getcode("int ffd; if (ffd<0 && (ffd=a)<0){}", "ffd"));

        // if (ticket #2442)
        ASSERT_EQUALS(";;;;if(!var){;}ifv{}", getcode("char *s; int x = 0; if (!s) { x = 2; } if (x) { }", "s"));
        ASSERT_EQUALS(";;;;if(!var){;}if{}", getcode("char *s; int x = 0; if (!s) { x = 2; } if (y) { }", "s"));

        // switch..
        ASSERT_EQUALS(";;switch{case;;break;};", getcode("char *s; switch(a){case 1: break;};", "s"));

        // loop..
        ASSERT_EQUALS(";;loop{}", getcode("char *s; while (a) { }", "s"));
        ASSERT_EQUALS(";;loopcallfunc{}", getcode("char *s; while (a()) { }", "s"));
        ASSERT_EQUALS(";;loop{}", getcode("char *s; for (a;b;c) { }", "s"));
        ASSERT_EQUALS(";;loop{alloc;}", getcode("char *s; for (a;b;c) { s=malloc(10); }", "s"));
        ASSERT_EQUALS(";;do{}loop;", getcode("char *s; do { } while (a);", "s"));
        ASSERT_EQUALS(";;while1{}", getcode("char *s; while(true) { }", "s"));
        ASSERT_EQUALS(";;while1{}", getcode("char *s; for(;;) { }", "s"));
        ASSERT_EQUALS(";;while(var){}", getcode("char *s; while (s) { }", "s"));
        ASSERT_EQUALS(";;while(!var){}", getcode("char *s; while (!s) { }", "s"));
        ASSERT_EQUALS(";;alloc;while(var){}", getcode("int fd = open(a,b); while (fd >= 0) { }", "fd"));
        ASSERT_EQUALS(";;alloc;while(!var){}", getcode("int fd = open(a,b); while (fd < 0) { }", "fd"));

        // asprintf..
        ASSERT_EQUALS(";;alloc;", getcode("char *s; asprintf(&s, \"xyz\");", "s"));
        ASSERT_EQUALS(";;alloc;", getcode("char *s; asprintf(&s, \"s: %s\", s);", "s"));
        ASSERT_EQUALS(";;;", getcode("char *s; asprintf(&p, \"s: %s\", s);", "s"));

        // Since we don't check how the return value is used we must bail out
        ASSERT_EQUALS("", getcode("char *s; int ret = asprintf(&s, \"xyz\");", "s"));
        TODO_ASSERT_EQUALS(";;alloc;",
                           "", getcode("char *s; int ret; ret=asprintf(&s, \"xyz\"); if (ret==-1) return;", "s"));

        // use..
        ASSERT_EQUALS(";;use;", getcode("char *s; a(s);", "s"));
        ASSERT_EQUALS(";;use;", getcode("char *s; (*a)(s);", "s"));
        ASSERT_EQUALS(";;use;", getcode("char *s; abc.a(s);", "s"));
        ASSERT_EQUALS(";;use;", getcode("char *s; s2 = s;", "s"));
        ASSERT_EQUALS(";;use;", getcode("char *s; s2 = s + 10;", "s"));
        ASSERT_EQUALS(";;use;", getcode("char *s; s2 = x + s;", "s"));
        ASSERT_EQUALS(";;use;if{;}", getcode("char *s; if (foo(s)) ;", "s"));
        ASSERT_EQUALS(";;use;", getcode("char *s; map1[s] = 0;", "s"));
        ASSERT_EQUALS(";;;use;", getcode("char *p; const char *q; q = p;", "p"));
        ASSERT_EQUALS(";;use;;", getcode("char *s; x = {1,s};", "s"));
        ASSERT_EQUALS(";{};;alloc;;use;", getcode("struct Foo { }; Foo *p; p = malloc(10); const Foo *q; q = p;", "p"));
        ASSERT_EQUALS(";;alloc;use;", getcode("Fred *fred; p.setFred(fred = new Fred);", "fred"));
        ASSERT_EQUALS(";;useuse_;", getcode("struct AB *ab; f(ab->a);", "ab"));
        ASSERT_EQUALS(";;use;", getcode("struct AB *ab; ab = pop(ab);", "ab"));

        // non-use..
        ASSERT_EQUALS(";;use_;", getcode("char *s; c = x + s[0];","s"));
        ASSERT_EQUALS(";;use_;", getcode("char *s; c = s[0] + x;","s"));
        ASSERT_EQUALS(";;use_;", getcode("type *c; y = x + c->y;","c"));
        ASSERT_EQUALS(";;use_;", getcode("type *c; y = c->y + x;","c"));
        ASSERT_EQUALS(";;use_;", getcode("char *s; s = s + 1;", "s"));
        ASSERT_EQUALS(";;dealloc;;", getcode("struct foo *s; free(s); printf(a,sizeof(*s));", "s"));
        ASSERT_EQUALS(";;do{dealloc;;}while(var);", getcode("struct foo *s; do{free(s); printf(a,sizeof(*s));}while(s);", "s"));

        // use reference
        ASSERT_EQUALS(";;callfunc&use;", getcode("struct AB *ab; f(&ab);", "ab"));

        // return..
        ASSERT_EQUALS(";;return;", getcode("char *s; return;", "s"));
        ASSERT_EQUALS(";;returnuse;", getcode("char *s; return s;", "s"));
        ASSERT_EQUALS(";;return;", getcode("char *s; return 5 + s[0];", "s"));

        // assign..
        ASSERT_EQUALS(";;assign;", getcode("char *s; s = 0;", "s"));
        ASSERT_EQUALS(";;;", getcode("char *s; s = strcpy(s, p);", "s"));

        // callfunc..
        ASSERT_EQUALS(";;assign;", getcode("char *s; s = a();", "s"));
        ASSERT_EQUALS(";;callfunc;", getcode("char *s; a();", "s"));
        ASSERT_EQUALS(";;callfunc;", getcode("char *s; abc.a();", "s"));
        ASSERT_EQUALS(";;;", getcode("char *s; x = a();", "s"));   // the function call is irrelevant

        // exit..
        ASSERT_EQUALS(";;exit;", getcode("char *s; exit(0);", "s"));
        ASSERT_EQUALS(";;callfunc;", getcode("char *s; _exit(0);", "s")); // not in std.cfg nor in gtk.cfg
        ASSERT_EQUALS(";;exit;", getcode("char *s; abort();", "s"));
        ASSERT_EQUALS(";;callfunc;", getcode("char *s; err(0);", "s")); // not in std.cfg nor in gtk.cfg
        ASSERT_EQUALS(";;if{exit;}", getcode("char *s; if (a) { exit(0); }", "s"));

        // list_for_each
        ASSERT_EQUALS(";;exit;{}", getcode("char *s; list_for_each(x,y,z) { }", "s"));

        // open/close
        ASSERT_EQUALS(";;alloc;if(var){dealloc;}", getcode("int f; f=open(a,b); if(f>=0)close(f);", "f"));
        ASSERT_EQUALS(";;alloc;if(var){dealloc;}", getcode("int f; f=open(a,b); if(f>-1)close(f);", "f"));
        ASSERT_EQUALS(";;alloc;ifv{;}", getcode("int f; f=open(a,b); if(f!=-1 || x);", "f"));
        ASSERT_EQUALS(";;;dealloc;loop{}", getcode(";int f; while (close(f) == -1) { }", "f"));
        ASSERT_EQUALS(";;;dealloc;assign;;", getcode(";int res; res = close(res);", "res"));

        ASSERT_EQUALS(";;dealloc;", getcode("int f; e |= fclose(f);", "f"));
        ASSERT_EQUALS(";;dealloc;", getcode("int f; e += fclose(f);", "f"));
        ASSERT_EQUALS(";;dealloc;", getcode("int f; foo(fclose(f));", "f"));

        // fcloseall..
        ASSERT_EQUALS(";;alloc;;", getcode("char *s; s = malloc(10); fcloseall();", "s"));
        ASSERT_EQUALS(";;alloc;dealloc;", getcode("FILE *f; f = fopen(a,b); fcloseall();", "f"));

        // call memcpy in class function..
        ASSERT_EQUALS(";;alloc;;", getcode("char *s; s = new char[10]; memcpy(s,a);", "s", true));

        // #2112 - Segmentation fault in the getcode function
        ASSERT_THROW(getcode("page *one = foo();\n"
                             "ASSERT(one, return 0)\n"
                             "const int two = rand();\n"
                             "return 0;\n"
                             "}", "one"), InternalError);

        // ticket #2336: calling member function with same name as a white_list function
        ASSERT_EQUALS(";;use;", getcode("char *s; foo.write(s);", "s"));

        // #2473 - inner struct
        ASSERT_EQUALS(";;alloc;{;;};dealloc;",
                      getcode("char *s = new char[10];\n"
                              "struct ab { int a, b; };\n"
                              "delete [] s;\n", "s"));

        // #4405 - catch
        ASSERT_EQUALS(";;catch{}", getcode("char *s; catch(err) { }", "s"));
    }

    bool test_white_list(const std::string& str, bool cpp = true) const {
        return CheckMemoryLeakInFunction::test_white_list(str, &settings1, cpp);
    }

    void call_func() const {
        // whitelist..
        ASSERT_EQUALS(true, test_white_list("qsort"));
        ASSERT_EQUALS(true, test_white_list("scanf"));
        ASSERT_EQUALS(true, test_white_list("sscanf"));

        // #1293
        ASSERT_EQUALS(true, test_white_list("time"));
        ASSERT_EQUALS(true, test_white_list("asctime"));
        ASSERT_EQUALS(true, test_white_list("asctime_r"));
        ASSERT_EQUALS(true, test_white_list("ctime"));
        ASSERT_EQUALS(true, test_white_list("ctime_r"));
        ASSERT_EQUALS(true, test_white_list("gmtime"));
        ASSERT_EQUALS(true, test_white_list("gmtime_r"));
        ASSERT_EQUALS(true, test_white_list("localtime"));
        ASSERT_EQUALS(true, test_white_list("localtime_r"));
        ASSERT_EQUALS(true, test_white_list("memcmp"));
        ASSERT_EQUALS(true, test_white_list("gets"));
        ASSERT_EQUALS(true, test_white_list("vprintf"));
        ASSERT_EQUALS(true, test_white_list("vfprintf"));
        ASSERT_EQUALS(true, test_white_list("vsprintf"));
        ASSERT_EQUALS(true, test_white_list("snprintf"));
        ASSERT_EQUALS(true, test_white_list("vsnprintf"));

        ASSERT_EQUALS(true, test_white_list("delete", true));
        ASSERT_EQUALS(false, test_white_list("delete", false));

        static const char * const call_func_white_list[] = {
            "access", "asprintf", "atof", "atoi", "atol", "chdir", "chmod", "clearerr", "chown"
            , "fchmod", "fcntl", "fdatasync", "feof", "ferror", "fflush", "fgetc", "fgetpos", "fgets"
            , "flock", "for", "fprintf", "fputc", "fputs", "fread", "free", "freopen", "fscanf", "fseek"
            , "fseeko", "fsetpos", "fstat", "fsync", "ftell", "ftello", "ftruncate"
            , "fwrite", "getc", "if", "ioctl", "lockf", "lseek", "open", "memchr", "memcpy"
            , "memmove", "memset", "mkstemp", "perror", "posix_fadvise", "posix_fallocate", "pread"
            , "printf", "puts", "pwrite", "read", "readahead", "readdir", "readdir_r", "readv"
            , "realloc", "return", "rewind", "rewinddir", "scandir", "seekdir"
            , "setbuf", "setbuffer", "setlinebuf", "setvbuf", "snprintf", "sprintf", "stpcpy", "strcasecmp"
            , "strcat", "strchr", "strcmp", "strcpy", "stricmp", "strlen", "strncat", "strncmp"
            , "strncpy", "strrchr", "strspn" ,"strstr", "strtod", "strtol", "strtoul", "switch"
            , "sync_file_range", "telldir", "typeid", "while", "write", "writev", "lstat", "stat"
            , "_open", "_wopen", "vscanf", "vsscanf", "vfscanf", "vasprintf", "utime", "utimes", "unlink"
            , "tempnam", "system", "symlink", "strpbrk", "strncasecmp", "strdup", "strcspn", "strcoll"
            , "setlocale", "sethostname", "rmdir", "rindex", "rename", "remove", "adjtime", "creat", "execle"
            , "execl", "execlp", "execve", "execv", "fmemopen", "fnmatch", "fopencookie", "fopen"
            , "getgrnam", "gethostbyaddr", "getnetbyname", "getopt", "getopt_long", "getprotobyname", "getpwnam"
            , "getservbyname", "getservbyport", "glob", "index", "inet_addr", "inet_aton", "inet_network"
            , "initgroups", "link", "mblen", "mbstowcs", "mbtowc", "mkdir", "mkfifo", "mknod", "obstack_printf"
            , "obstack_vprintf", "opendir", "parse_printf_format", "pathconf", "popen", "psignal"
            , "readlink", "regcomp", "strxfrm", "wordexp", "sizeof", "strtok"
        };

        for (unsigned int i = 0; i < (sizeof(call_func_white_list) / sizeof(char *)); ++i) {
            bool ret = test_white_list(call_func_white_list[i]);
            ASSERT_EQUALS("", ret ? "" : call_func_white_list[i]);
        }
    }


    std::string simplifycode(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;

        // Tokenize..
        std::istringstream istr(code);
        TokenList list(&settings);
        list.createTokens(istr,"test.cpp");
        Token *tokens=list.front();

        // replace "if ( ! var )" => "if(!var)"
        for (Token *tok = tokens; tok; tok = tok->next()) {
            if (Token::Match(tok, "if|while ( var )")) {
                Token::eraseTokens(tok, tok->tokAt(4));
                tok->str(tok->str() + "(var)");
            }

            else if (Token::Match(tok, "if|while ( ! var )")) {
                Token::eraseTokens(tok, tok->tokAt(5));
                tok->str(tok->str() + "(!var)");
            }
        }

        CheckMemoryLeakInFunction checkMemoryLeak(nullptr, &settings, this);
        checkMemoryLeak.simplifycode(tokens);

        return list.front()->stringifyList(0, false);
    }


    // Test that the CheckMemoryLeaksInFunction::simplifycode works
    void simplifycode() {
        ASSERT_EQUALS(";", simplifycode("; ; ; ;"));
        ASSERT_EQUALS(";", simplifycode("; if ;"));
        ASSERT_EQUALS("alloc ;", simplifycode("alloc ; if ; if(var) ; ifv ; if(!var) ;"));
        ASSERT_EQUALS("alloc ;", simplifycode("alloc ; if ; else ;"));

        // use..
        ASSERT_EQUALS("; use ; }", simplifycode("; use use ; }"));
        ASSERT_EQUALS("; use ; }", simplifycode("; use use_ ; }"));
        ASSERT_EQUALS("; use ; }", simplifycode("; use_ use ; }"));
        ASSERT_EQUALS("; use ; }", simplifycode("; &use use ; }"));
        ASSERT_EQUALS("; use ; }", simplifycode("; use &use ; }"));
        ASSERT_EQUALS("; alloc ; dealloc ; }", simplifycode("; alloc ; use ; use ; if use ; dealloc ; }"));

        // if, else..
        ASSERT_EQUALS("; alloc ; if break ; dealloc ;", simplifycode("; alloc ; if { break; } dealloc ;"));
        ASSERT_EQUALS("; alloc ; if continue ; dealloc ;", simplifycode("; alloc ; if { continue; } dealloc ;"));
        ASSERT_EQUALS("; alloc ;", simplifycode("; alloc; if { return use; }"));
        ASSERT_EQUALS("; alloc ; dealloc ;", simplifycode("; alloc; if(!var) { return; } dealloc;"));
        ASSERT_EQUALS("; alloc ;", simplifycode("; if { alloc; } else { return; }"));
        ASSERT_EQUALS("; alloc ; dealloc ;", simplifycode("; alloc ; if(!var) { alloc ; } dealloc ;"));
        ASSERT_EQUALS("; use ;", simplifycode("; if(var) use ;"));
        ASSERT_EQUALS("; break ;", simplifycode("; if break ; else break ;"));
        ASSERT_EQUALS("; alloc ; if return ;", simplifycode("; alloc ; loop { if return ; if continue ; }"));
        ASSERT_EQUALS("; alloc ; if return ;", simplifycode("; alloc ; loop { if continue ; else return ; }"));

        ASSERT_EQUALS("; alloc ; if dealloc ;", simplifycode("; alloc ; if(!var) { return ; } if { dealloc ; }"));
        ASSERT_EQUALS("; if alloc ; else assign ; return use ;", simplifycode("; callfunc ; if callfunc { alloc ; } else { assign ; } return use ;"));

        ASSERT_EQUALS("; dealloc ; return ;", simplifycode("; while1 { if callfunc { dealloc ; return ; } else { continue ; } }"));

        // remove outer if (#2733)
        ASSERT_EQUALS("alloc ; return ; }", simplifycode("alloc ; if { if return use ; } return ; }"));
        ASSERT_EQUALS("alloc ; return ; }", simplifycode("alloc ; if { if(var) return use ; } return ; }"));
        ASSERT_EQUALS("alloc ; return ; }", simplifycode("alloc ; if(var) { if return use ; } return ; }"));

        // "if ; .."
        ASSERT_EQUALS("; if xxx ;", simplifycode("; if ; else xxx ;"));
        ASSERT_EQUALS("; if(var) xxx ;", simplifycode("; if(!var) ; else xxx ;"));
        ASSERT_EQUALS("; if(!var) xxx ;", simplifycode("; if(var) ; else xxx ;"));
        ASSERT_EQUALS("; ifv xxx ;", simplifycode("; ifv ; else xxx ;"));
        ASSERT_EQUALS("; alloc ;", simplifycode("; alloc; if { dealloc; return; }"));
        ASSERT_EQUALS("; alloc ;", simplifycode("; alloc; if { return use; }"));
        ASSERT_EQUALS("; alloc ; return ;", simplifycode(";alloc;if{return;}return;"));
        ASSERT_EQUALS("; alloc ; if assign ; dealloc ;", simplifycode(";alloc;if{assign;}dealloc;"));

        // if(var)
        ASSERT_EQUALS("; alloc ; return use ;", simplifycode("; alloc ; return use ;"));
        ASSERT_EQUALS("; alloc ; return use ;", simplifycode("; alloc ; ifv return ; return use ;"));

        // switch..
        ASSERT_EQUALS("; alloc ; dealloc ;", simplifycode(";alloc;switch{case;break;};dealloc;"));
        ASSERT_EQUALS(";", simplifycode("; switch { case ; return ; default ; break ; }"));
        ASSERT_EQUALS(";", simplifycode("; switch { case ; if { return ; } break ; default ; break ; }"));
        ASSERT_EQUALS("; use ;", simplifycode("; switch { case ; return ; default ; use ; break ; }"));
        ASSERT_EQUALS("; use ;", simplifycode("; while1 { loop { ; } switch { case ; dealloc ; return ; default ; break ; } }"));
        ASSERT_EQUALS("; { dealloc ; return ; } }", simplifycode("switch { case ; case ; dealloc ; return ; default ; dealloc ; return ; } }"));

        // loops..
        ASSERT_EQUALS(";", simplifycode("; loop { ; }"));
        ASSERT_EQUALS(";", simplifycode("; loop { break; }"));
        ASSERT_EQUALS(";", simplifycode("; loop { if { break; } }"));
        ASSERT_EQUALS("; loop alloc ;", simplifycode("; loop { alloc ; }"));
        ASSERT_EQUALS("; alloc ; alloc ;", simplifycode("; alloc ; do { alloc ; } loop ;"));
        ASSERT_EQUALS("; exit ;", simplifycode("; alloc ; do { } loop ; exit ;"));
        ASSERT_EQUALS("; loop use ;", simplifycode("; loop { loop loop use ; } ;"));
        ASSERT_EQUALS("; }", simplifycode("; loop { if break ; break ; } ; }"));
        ASSERT_EQUALS("; }", simplifycode("; loop { if continue ; if continue ; } ; }"));
        {
            // ticket #3267
            const char expected[] = "; loop if alloc ; if { dealloc ; return ; } }";
            ASSERT_EQUALS(expected, simplifycode("; loop { if alloc ; } if { dealloc ; return ; } }"));
            ASSERT_EQUALS(expected, simplifycode("; loop { if { alloc ; if(!var) { return ; } } } if { dealloc ; return ; } }"));
        }

        ASSERT_EQUALS("; alloc ;", simplifycode("; alloc ; while(!var) alloc ;"));

        ASSERT_EQUALS("; alloc ; dealloc ; return ;", simplifycode("; alloc ; while1 { if { dealloc ; return ; } }"));
        ASSERT_EQUALS("; alloc ; dealloc ; return ;", simplifycode("; alloc ; while1 { if { dealloc ; return ; } if { continue ; } }"));
        ASSERT_EQUALS("; alloc ;", simplifycode("; alloc ; while1 { if { dealloc ; return ; } if { break ; } }"));
        ASSERT_EQUALS("; alloc ; use ; }", simplifycode("; alloc ; while1 { if { dealloc ; return ; } continue ; } ; }"));

        ASSERT_EQUALS(";", simplifycode("; do { dealloc ; alloc ; } while(var) ;"));
        ASSERT_EQUALS("dealloc ; alloc ;", simplifycode("loop { dealloc ; alloc ; }"));
        ASSERT_EQUALS("dealloc ; alloc ;", simplifycode("while1 { dealloc ; alloc ; }"));
        ASSERT_EQUALS("use ; }", simplifycode("loop { use ; callfunc ; } }"));

        ASSERT_EQUALS(";", simplifycode("; loop { if { continue ; } else { if continue ; } }"));
        ASSERT_EQUALS(";", simplifycode("; loop { { if continue ; if continue ; } }"));

        ASSERT_EQUALS("; use ;", simplifycode("; while1 { if { dealloc ; return ; } if { if { continue ; } } }"));

        // scope..
        TODO_ASSERT_EQUALS("; assign ; if alloc ; }",
                           "; assign ; dealloc ; if alloc ; }", simplifycode("; assign ; { dealloc ; if alloc ; } }"));

        // callfunc..
        ASSERT_EQUALS("; callfunc ; }", simplifycode(";callfunc;}"));
        ASSERT_EQUALS(";", simplifycode(";callfunc;;"));
        ASSERT_EQUALS("; callfunc ; }", simplifycode(";callfunc callfunc ; }"));
        ASSERT_EQUALS("dealloc ; alloc ; return ; }", simplifycode("while1 { dealloc ; alloc ; } callfunc ; return ; }"));
        ASSERT_EQUALS("; }", simplifycode("loop callfunc ; }"));
        ASSERT_EQUALS("alloc ; dealloc ; }", simplifycode("alloc ; if { dealloc ; callfunc } dealloc ; }")); // #4405

        // #2900 - don't report false positive
        ASSERT_EQUALS("; alloc ; if { if { dealloc ; callfunc ; } return ; } dealloc ; }",
                      simplifycode("; alloc ; if { if { dealloc ; callfunc ; } return ; } dealloc ; }"));

        // exit..
        ASSERT_EQUALS("; exit ;", simplifycode("; alloc; exit;"));
        ASSERT_EQUALS("; exit ;", simplifycode("; alloc; if { loop ; } dealloc; exit;"));
        ASSERT_EQUALS(";", simplifycode("; if { alloc; exit; }"));
        ASSERT_EQUALS("; alloc ;", simplifycode("; alloc ; if { use; exit; }"));
        ASSERT_EQUALS("; alloc ;", simplifycode("; alloc ; if(!var) { exit; }"));
        TODO_ASSERT_EQUALS(";",
                           "; if(var) exit ;", simplifycode("; alloc ; if(var) { exit; }"));
        TODO_ASSERT_EQUALS(";\n; alloc ;",
                           "; alloc ; ifv exit ;", simplifycode("; alloc ; ifv { exit; }"));

        // try-catch
        ASSERT_EQUALS("; }", simplifycode("; try ; catch exit ; }"));

        // dealloc; dealloc;
        ASSERT_EQUALS("; alloc ; if dealloc ; dealloc ;", simplifycode("; alloc ; if { dealloc ; } dealloc ;"));

        // use ; dealloc ;
        ASSERT_EQUALS("; alloc ; use ; if return ; dealloc ;", simplifycode("; alloc ; use ; if { return ; } dealloc ;"));

        // #2635 - false negative
        ASSERT_EQUALS("; alloc ; return use ; }",
                      simplifycode("; alloc ; if(!var) { loop { ifv { } } alloc ; } return use; }"));
    }



    // is there a leak in given code? if so, return the linenr
    static unsigned int dofindleak(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.debug = settings.debugwarnings = true;

        // Tokenize..
        std::istringstream istr(code);
        TokenList list(&settings);
        list.createTokens(istr,"test.cpp");
        Token *tokens=list.front();

        // replace "if ( ! var )" => "if(!var)"
        for (Token *tok = tokens; tok; tok = tok->next()) {
            if (tok->str() == "if_var") {
                tok->str("if(var)");
            }

            else if (Token::simpleMatch(tok, "if ( var )")) {
                Token::eraseTokens(tok, tok->tokAt(4));
                tok->str("if(var)");
            }

            else if (Token::simpleMatch(tok, "if ( ! var )")) {
                Token::eraseTokens(tok, tok->tokAt(5));
                tok->str("if(!var)");
            }
        }

        const Token *tok = CheckMemoryLeakInFunction::findleak(tokens);
        return (tok ? tok->linenr() : (unsigned int)(-1));
    }

    void findleak() const {
        static const unsigned int notfound = (unsigned int)(-1);

        ASSERT_EQUALS(1,  dofindleak("alloc;"));
        ASSERT_EQUALS(1,  dofindleak("; use; { alloc; }"));
        ASSERT_EQUALS(2,  dofindleak("alloc;\n return;"));
        ASSERT_EQUALS(notfound, dofindleak("alloc; return use;"));
        ASSERT_EQUALS(2, dofindleak("alloc;\n callfunc;"));
        ASSERT_EQUALS(notfound, dofindleak("alloc; use;"));
        ASSERT_EQUALS(notfound, dofindleak("assign; alloc; dealloc;"));
        ASSERT_EQUALS(notfound, dofindleak("assign; if alloc; dealloc;"));

        // if alloc..
        ASSERT_EQUALS(2,  dofindleak("if alloc;\n return;"));
        ASSERT_EQUALS(notfound, dofindleak("if alloc;\n return use;"));
        ASSERT_EQUALS(notfound, dofindleak("if alloc;\n use;"));
        ASSERT_EQUALS(notfound, dofindleak("if alloc;\n if assign;\n if dealloc; }"));

        // if..
        ASSERT_EQUALS(notfound, dofindleak("alloc; ifv dealloc;"));
        ASSERT_EQUALS(2,  dofindleak("alloc;\n if return;\n dealloc;"));
        ASSERT_EQUALS(2,  dofindleak("alloc;\n if continue;\n dealloc;"));
        ASSERT_EQUALS(2,  dofindleak("alloc;\n if_var return;\n dealloc;"));
        ASSERT_EQUALS(3,  dofindleak("alloc;\n if\n return;\n dealloc;"));
        ASSERT_EQUALS(notfound, dofindleak("alloc; if { dealloc ; return; } dealloc;"));
        ASSERT_EQUALS(notfound, dofindleak("alloc; if { dealloc ; return; } dealloc;"));
        ASSERT_EQUALS(notfound, dofindleak("alloc; if { dealloc ; alloc; } dealloc;"));
        ASSERT_EQUALS(notfound, dofindleak("alloc;\n if(!var)\n { callfunc;\n return;\n }\n use;"));

        ASSERT_EQUALS(notfound, dofindleak("alloc; if { return use; } dealloc;"));
        ASSERT_EQUALS(notfound, dofindleak("alloc; if { dealloc; return; } dealloc;"));

        ASSERT_EQUALS(5, dofindleak("{\n;\n alloc;\n if dealloc;\n}"));

        // assign..
        ASSERT_EQUALS(2,  dofindleak("alloc;\n assign;\n dealloc;"));
        ASSERT_EQUALS(notfound, dofindleak("alloc;\n if(!var) assign;\n dealloc;"));
        ASSERT_EQUALS(2,  dofindleak(";alloc;\n if assign;\n dealloc;"));

        // loop..
        TODO_ASSERT_EQUALS(1, notfound, dofindleak("; loop { alloc ; if break; dealloc ; }"));
        TODO_ASSERT_EQUALS(1, notfound, dofindleak("; loop { alloc ; if continue; dealloc ; }"));
        ASSERT_EQUALS(notfound, dofindleak("; loop { alloc ; if break; } dealloc ;"));
        ASSERT_EQUALS(1, dofindleak("; loop alloc ;"));
        ASSERT_EQUALS(1, dofindleak("; loop alloc ; dealloc ;"));

        // callfunc (might be noreturn)
        ASSERT_EQUALS(notfound, dofindleak("; alloc ; callfunc ; }"));
    }


    void simple5() {
        check("static char *f()\n"
              "{\n"
              "    struct *str = new strlist;\n"
              "    return &str->s;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void simple7() {
        // A garbage collector may delete f automatically
        check("class Fred;\n"
              "void foo()\n"
              "{\n"
              "    Fred *f = new Fred;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void simple9() {
        check("void foo()\n"
              "{\n"
              "    MyClass *c = new MyClass();\n"
              "    c->free(c);\n"
              "    delete c;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void simple11() {
        check("void Fred::aaa()\n"
              "{ }\n"
              "\n"
              "void Fred::foo()\n"
              "{\n"
              "    char *s = NULL;\n"
              "    if (a)\n"
              "        s = malloc(10);\n"
              "    else if (b)\n"
              "        s = malloc(10);\n"
              "    else\n"
              "        f();\n"
              "    g(s);\n"
              "    if (c)\n"
              "        h(s);\n"
              "    free(s);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void Fred::aaa()\n"
              "{ }\n"
              "\n"
              "void Fred::foo()\n"
              "{\n"
              "    gchar *s = NULL;\n"
              "    if (a)\n"
              "        s = g_malloc(10);\n"
              "    else if (b)\n"
              "        s = g_malloc(10);\n"
              "    else\n"
              "        f();\n"
              "    g(s);\n"
              "    if (c)\n"
              "        h(s);\n"
              "    g_free(s);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void nonstd_free() {
        check("void f() {\n"
              "    void* mem = malloc(100, foo);" // Non-standard malloc() implementation
              "    free(mem, bar);" // Non-standard free() implementation (#5665)
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void new_nothrow() {
        check("void f()\n"
              "{\n"
              "    int *p = new(std::nothrow) int;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: p\n", errout.str());

        check("void f()\n"
              "{\n"
              "    using std::nothrow;\n"
              "    int *p = new(nothrow) int;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: p\n", errout.str());

        check("void f()\n"
              "{\n"
              "    int *p = new(std::nothrow) int[10];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: p\n", errout.str());

        check("void f()\n"
              "{\n"
              "    using namespace std;\n"
              "    int *p = new(nothrow) int[10];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: p\n", errout.str());

        check("void f()\n"
              "{\n"
              "    int *p = new(std::nothrow) int;\n"
              "    delete [] p;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Mismatching allocation and deallocation: p\n", errout.str());

        check("void f()\n"
              "{\n"
              "    int *p = new(std::nothrow) int[10];\n"
              "    delete p;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Mismatching allocation and deallocation: p\n", errout.str());

        check("void f()\n"
              "{\n"
              "    Fred *f = new(nothrow) Fred;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    Fred *f = new(std::nothrow) Fred;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // ticket #2971
        check("void f()\n"
              "{\n"
              "    Fred *f = new(std::nothrow) Fred[10];\n"
              "    delete f;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Mismatching allocation and deallocation: f\n", errout.str());

        check("void f()\n"
              "{\n"
              "    struct Fred *f = new(std::nothrow) struct Fred[10];\n"
              "    delete f;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Mismatching allocation and deallocation: f\n", errout.str());
    }


    void staticvar() {
        check("int f()\n"
              "{\n"
              "    static char *s = 0;\n"
              "    free(s);\n"
              "    s = malloc(100);\n"
              "    return 123;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("int f()\n"
              "{\n"
              "    static gchar *s = 0;\n"
              "    g_free(s);\n"
              "    s = g_malloc(100);\n"
              "    return 123;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void externvar() {
        check("void f()\n"
              "{\n"
              "    extern char *s;\n"
              "    s = malloc(100);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void referencevar() {  // 3954 - false positive for reference pointer
        check("void f() {\n"
              "    char *&x = get();\n"
              "    x = malloc(100);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void alloc_alloc_1() {
        check("void foo()\n"
              "{\n"
              "    char *str;\n"
              "    str = new char[10];\n"
              "    str = new char[20];\n"
              "    delete [] str;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: str\n", errout.str());
    }

    void ifelse6() {
        check("static char *f()\n"
              "{\n"
              "    char *s = new char[100];\n"
              "    if ( a == b )\n"
              "    {\n"
              "        return s;\n"
              "    }\n"
              "    return NULL;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Memory leak: s\n", errout.str());
    }


    void ifelse7() {
        check("static char *f()\n"
              "{\n"
              "    char *s;\n"
              "    if ( abc )\n"
              "    {\n"
              "        s = new char[10];\n"
              "    }\n"
              "    return s;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void ifelse8() {
        check("static char *f()\n"
              "{\n"
              "    char *s = new char[10];\n"
              "    if ( s )\n"
              "    {\n"
              "        return s;\n"
              "    }\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ifelse10() {
        Settings settings;
        settings.experimental = true;
        check("static char *f()\n"
              "{\n"
              "    char *s = new char[10];\n"
              "    if ( ghfgf )\n"
              "    {\n"
              "        str[0] = s;\n"
              "    }\n"
              "    else\n"
              "    {\n"
              "        str[0] = s;\n"
              "    }\n"
              "}\n", &settings);
        ASSERT_EQUALS("", errout.str());
    }



    void if4() {
        check("void f()\n"
              "{\n"
              "    char *s;\n"
              "    bool b = true;\n"
              "    if (b && (s = malloc(256)))\n"
              "        ;\n"
              "    if (b)\n"
              "        free(s);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void f()\n"
              "{\n"
              "    gchar *s;\n"
              "    bool b = true;\n"
              "    if (b && (s = g_malloc(256)))\n"
              "        ;\n"
              "    if (b)\n"
              "        g_free(s);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void if7() {
        check("void f( bool b )\n"
              "{\n"
              "    int *a=0;\n"
              "    if( b )\n"
              "    {\n"
              "        a = new int[10];\n"
              "    }\n"
              "\n"
              "    if( b )\n"
              "        delete [] a;\n"
              "    else {}\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void if8() {
        check("static void f(int i)\n"
              "{\n"
              "    char *c = malloc(50);\n"
              "    if (i == 1)\n"
              "    {\n"
              "        free(c);\n"
              "        return;\n"
              "    }\n"
              "    if (i == 2)\n"
              "    {\n"
              "        return;\n"
              "    }\n"
              "    free(c);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11]: (error) Memory leak: c\n", errout.str());
        check("static void f(int i)\n"
              "{\n"
              "    gchar *c = g_malloc(50);\n"
              "    if (i == 1)\n"
              "    {\n"
              "        g_free(c);\n"
              "        return;\n"
              "    }\n"
              "    if (i == 2)\n"
              "    {\n"
              "        return;\n"
              "    }\n"
              "    g_free(c);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11]: (error) Memory leak: c\n", errout.str());
    }

    void if9() {
        check("static void f()\n"
              "{\n"
              "    char *buf = NULL, *tmp;\n"
              "    if (!(tmp = realloc(buf, 50)))\n"
              "    {\n"
              "        free(buf);\n"
              "        return NULL;\n"
              "    }\n"
              "    buf = tmp;\n"
              "    return buf;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void if10() {
        check("static void f()\n"
              "{\n"
              "    char *buf = malloc(10);\n"
              "    if (aa)\n"
              "        ;\n"
              "    else if (buf = realloc(buf, 100))\n"
              "        ;\n"
              "    free(buf);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Common realloc mistake: \'buf\' nulled but not freed upon failure\n", errout.str());
    }

    void if11() {
        Settings settings;
        settings.experimental = true;
        check("void foo()\n"
              "{\n"
              "    int *x = new int[10];\n"
              "    if (x == 0 || aa)\n"
              "    {\n"
              "        return 1;\n"
              "    }\n"
              "    delete [] x;\n"
              "}\n", &settings);
        TODO_ASSERT_EQUALS("[test.cpp:6]: (error) Memory leak: x\n",
                           "", errout.str());
    }


    void forwhile5() {
        check("void f(const char **a)\n"
              "{\n"
              "    char *str = 0;\n"
              "    for (int i = 0; i < 10 && !str; ++i)\n"
              "    {\n"
              "        str = strdup(a[i]);\n"
              "    }\n"
              "    return str;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void forwhile6() {
        check("void f(const char **a)\n"
              "{\n"
              "    char *str = 0;\n"
              "    for (int i = 0; i < 10 && !str; ++i)\n"
              "    {\n"
              "        str = strdup(a[i]);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Memory leak: str\n", errout.str());
    }


    void forwhile8() {
        check("char *f()\n"
              "{\n"
              "    char *a = 0;\n"
              "    int i = 0;\n"
              "    for( ;; )\n"
              "    {\n"
              "    ++i;\n"
              "    a = realloc( a, i );\n"
              "    if( !a )\n"
              "        return 0;\n"
              "\n"
              "    if( i > 10 )\n"
              "        break;\n"
              "    }\n"
              "\n"
              "    return a;\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:10]: (error) Memory leak: a\n",

                           "[test.cpp:8]: (error) Common realloc mistake: \'a\' nulled but not freed upon failure\n",
                           errout.str());
    }


    void forwhile9() {
        Settings settings;
        settings.experimental = true;
        check("char *f()\n"
              "{\n"
              "    char *a = 0;\n"
              "    int i = 0;\n"
              "    for(i = 0 ;i < 50 ; i++)\n"
              "    {\n"
              "        if(func1(i))\n"
              "            continue;\n"
              "        a = realloc( a, i );\n"
              "        if(func2(i))\n"
              "            continue;\n"
              "    }\n"
              "\n"
              "    return a;\n"
              "}\n", &settings);
        ASSERT_EQUALS("[test.cpp:9]: (error) Common realloc mistake: \'a\' nulled but not freed upon failure\n", errout.str());
    }


    void forwhile10() {
        Settings settings;
        settings.experimental = true;
        check("char *f()\n"
              "{\n"
              "    char *a = 0;\n"
              "    int i = 0;\n"
              "    for(i = 0; i < 50; i++)\n"
              "    {\n"
              "        if(func1(i))\n"
              "            continue;\n"
              "        a = realloc( a, i );\n"
              "        if(func2(i))\n"
              "            return;\n"
              "    }\n"
              "\n"
              "    return a;\n"
              "}\n", &settings);
        ASSERT_EQUALS("[test.cpp:9]: (error) Common realloc mistake: \'a\' nulled but not freed upon failure\n"
                      "[test.cpp:11]: (error) Memory leak: a\n", errout.str());
    }


    void forwhile11() {
        check("int main()\n"
              "{\n"
              "    FILE *stream=NULL;\n"
              "    while((stream = fopen(name,\"r\")) == NULL)\n"
              "    { }\n"
              "    if(stream!=NULL) fclose(stream);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }




    void switch2() {
        check("void f()\n"
              "{\n"
              "    char *str = new char[10];\n"
              "    switch (abc)\n"
              "    {\n"
              "        case 1:\n"
              "            delete [] str;\n"
              "            break;\n"
              "        default:\n"
              "            break;\n"
              "    };\n"
              "}");
        ASSERT_EQUALS("[test.cpp:12]: (error) Memory leak: str\n", errout.str());
    }

    void switch3() {
        check("void f()\n"
              "{\n"
              "    char *str = new char[10];\n"
              "    while (abc)\n"
              "    {\n"
              "        switch (def)\n"
              "        {\n"
              "            default:\n"
              "                return;\n"
              "        }\n"
              "    }\n"
              "    delete [] str;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (error) Memory leak: str\n", errout.str());
    }

    void ret5() {
        check("static char * f()\n"
              "{\n"
              "    char *c = new char[50];\n"
              "    return (c ? c : NULL);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ret6() {
        check("void foo()\n"
              "{\n"
              "    char *c = new char[50];\n"
              "    return strcpy(c, \"foo\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ret7() {
        check("void foo()\n"
              "{\n"
              "    char *c = new char[50];\n"
              "    return memcpy(c, \"foo\",4);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ret8() {
        check("char *foo()\n"
              "{\n"
              "    char *c = new char[50];\n"
              "    return ((char *)(c+1));\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void mismatch1() {
        Settings settings;
        settings.experimental = true;

        check("void f()\n"
              "{\n"
              "    int *a = new int[10];\n"
              "    free(a);\n"
              "}\n", &settings);
        ASSERT_EQUALS("[test.cpp:4]: (error) Mismatching allocation and deallocation: a\n", errout.str());

        // ticket #2971
        check("void f()\n"
              "{\n"
              "    Fred *a = new Fred[10];\n"
              "    free(a);\n"
              "}\n", &settings);
        ASSERT_EQUALS("[test.cpp:4]: (error) Mismatching allocation and deallocation: a\n", errout.str());

        check("void f()\n"
              "{\n"
              "    struct Fred *a = new struct Fred[10];\n"
              "    free(a);\n"
              "}\n", &settings);
        ASSERT_EQUALS("[test.cpp:4]: (error) Mismatching allocation and deallocation: a\n", errout.str());
    }

    void mismatch2() {
        check("void f()\n"
              "{\n"
              "    FILE *fp;\n"
              "\n"
              "    fp = fopen();\n"
              "    fclose(fp);\n"
              "\n"
              "    fp = popen();\n"
              "    pclose(fp);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void mismatch3() {
        check("void f()\n"
              "{\n"
              "    FILE *fp;\n"
              "\n"
              "    if (abc) fp = fopen();\n"
              "    else fp = popen();\n"
              "\n"
              "    if (abc) fclose(fp);\n"
              "    else pclose(fp);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void mismatch4() {
        check("void f()\n"
              "{\n"
              "    char *p = 0;\n"
              "    for (i = 0; i < 10; ++i)\n"
              "    {\n"
              "        delete p;\n"
              "        p = new char[100];\n"
              "    }\n"
              "    delete [] p;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (error) Mismatching allocation and deallocation: p\n", errout.str());
    }

    void mismatch5() {
        check("void f() {\n"
              "    C *c = new C;\n"
              "    delete c;\n"
              "    c = new C[2];\n"
              "    delete [] c;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void mismatch6() { // #4599
        check("void test ( int do_gzip , const char * fn ) {\n"
              "    FILE * f ;\n"
              "    if ( do_gzip ) {\n"
              "        f = popen ( fn , \"wb\" ) ;\n"
              "    } else {\n"
              "        f = fopen ( fn , \"wb\" ) ;\n"
              "    }\n"
              "    if ( do_gzip ) {\n"
              "        pclose ( f ) ; }\n"
              "    else {\n"
              "        fclose ( f ) ; }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void mismatchSize() {
        check("void f(char *buf)\n"
              "{\n"
              "    int i;\n"
              "    buf = malloc(3);\n"
              "    buf[i] = 0;\n"
              "    free(buf);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void f(gchar *buf)\n"
              "{\n"
              "    int i;\n"
              "    buf = g_malloc(3);\n"
              "    buf[i] = 0;\n"
              "    g_free(buf);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    ////////////////////////////////////////////////
    // function calls
    ////////////////////////////////////////////////


    void func3() {
        check("static void foo(const char *str)\n"
              "{ }\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    char *p = new char[100];\n"
              "    foo(p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Memory leak: p\n", errout.str());
    }


    void func4() {
        check("static void foo(char *str)\n"
              "{\n"
              "    delete [] str;\n"
              "}\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    char *p = new char[100];\n"
              "    foo(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void func5() {
        Settings settings;
        settings.experimental = true;

        check("static void foo(char *str)\n"
              "{\n"
              "    delete str;\n"
              "}\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    char *p = new char[100];\n"
              "    foo(p);\n"
              "}\n", &settings);
        ASSERT_EQUALS("[test.cpp:9] -> [test.cpp:3]: (error) Mismatching allocation and deallocation: str\n",
                      errout.str());
    }


    void func6() {
        check("static void foo(char *str)\n"
              "{\n"
              "    goto abc;\n"
              "}\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    char *p = new char[100];\n"
              "    foo(p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (error) Memory leak: p\n", errout.str());
    }


    void func7() {
        check("static void foo(char *str)\n"
              "{\n"
              "    if (abc)\n"
              "        return;"
              "    delete [] str;\n"
              "}\n"
              "\n"
              "static void f()\n"
              "{\n"
              "    char *p = new char[100];\n"
              "    foo(p);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:11]: (error) Memory leak: p\n",
                           "", errout.str());
    }


    void func9() {
        check("int b()\n"
              "{\n"
              "    return 0;\n"
              "}\n"
              "\n"
              "void a()\n"
              "{\n"
              "    char *a = new char[10];\n"
              "    if (b())\n"
              "        return;\n"
              "    delete [] a;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void func10() {
        check("static void f(void (*fnc)(char*))\n"
              "{\n"
              "    char *c = malloc(50);\n"
              "    (fnc)(c);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void func11() {
        check("static void f(struct1 *s1)\n"
              "{\n"
              "    char *c = malloc(50);\n"
              "    (s1->fnc)(c);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void func12() {
        check("void add_list(struct mmtimer *n)\n"
              "{\n"
              "    rb_link_node(&n->list, parent, link);\n"
              "}\n"
              "\n"
              "int foo()\n"
              "{\n"
              "    struct mmtimer *base;\n"
              "\n"
              "    base = kmalloc(sizeof(struct mmtimer), GFP_KERNEL);\n"
              "    if (base == NULL)\n"
              "        return -ENOMEM;\n"
              "\n"
              "    add_list(base);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void func13() {
        check("static void f()\n"
              "{\n"
              "    char *p = malloc(100);\n"
              "    foo(&p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void func14() {
        // It is not known what the "foo" that only takes one parameter does..
        check("static void foo(char *a, char *b)\n"
              "{\n"
              "    free(a);\n"
              "    free(b);\n"
              "}\n"
              "static void f()\n"
              "{\n"
              "    char *p = malloc(100);\n"
              "    foo(p);\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("static void foo(gchar *a, gchar *b)\n"
              "{\n"
              "    g_free(a);\n"
              "    g_free(b);\n"
              "}\n"
              "static void f()\n"
              "{\n"
              "    gchar *p = g_malloc(100);\n"
              "    foo(p);\n"
              "    g_free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void func15() {
        check("static void a()\n"
              "{ return true; }\n"
              "\n"
              "static void b()\n"
              "{\n"
              "    char *p = malloc(100);\n"
              "    if (a()) return;\n"    // <- memory leak
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Memory leak: p\n", errout.str());
        check("static void a()\n"
              "{ return true; }\n"
              "\n"
              "static void b()\n"
              "{\n"
              "    gchar *p = g_malloc(100);\n"
              "    if (a()) return;\n"    // <- memory leak
              "    g_free(p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Memory leak: p\n", errout.str());
    }

    void func16() {
        check("static void a( bo_t *p_bo)\n"
              "{\n"
              "    p_bo->buffer = realloc( p_bo->buffer, 100 );\n"
              "}\n"
              "\n"
              "static bo_t * b()\n"
              "{\n"
              "    bo_t *box;\n"
              "    if( ( box = malloc( sizeof( bo_t ) ) ) )\n"
              "    {\n"
              "        a(box);\n"
              "        a(box);\n"
              "    }\n"
              "    return box;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void func17() {
        // The "bar" function must be reduced to "use"

        check("bool bar(char **parent, char *res, bool a)\n"
              "{\n"
              "    if( a )\n"
              "    {\n"
              "        *parent = res;\n"
              "        return false;\n"
              "    }\n"
              "    return true;\n"
              "}\n"
              "\n"
              "void foo(char **parent, bool a)\n"
              "{\n"
              "    if (a)\n"
              "    {\n"
              "        char *res = malloc(65);\n"
              "        bar(parent, res, a);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void func18() {
        // No false positive
        // The "free_pointers" will deallocate all pointers
        check("static void free_pointers(int arg_count, ...)\n"
              "{\n"
              "    va_list a;\n"
              "    va_start(a, arg_count);\n"
              "    for (int i = 0; i < arg_count; i++)\n"
              "    {\n"
              "        free(va_arg(a, void *));\n"
              "    }\n"
              "    va_end(a);\n"
              "}\n"
              "\n"
              "static char* foo()\n"
              "{\n"
              "    return strdup(\"\");\n"
              "}\n"
              "\n"
              "static void bar()\n"
              "{\n"
              "    int *p = malloc(16);\n"
              "    free_pointers(1, p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("static void free_pointers(int arg_count, ...)\n"
              "{\n"
              "    va_list a;\n"
              "    va_start(a, arg_count);\n"
              "    for (int i = 0; i < arg_count; i++)\n"
              "    {\n"
              "        g_free(va_arg(a, void *));\n"
              "    }\n"
              "    va_end(a);\n"
              "}\n"
              "\n"
              "static gchar* foo()\n"
              "{\n"
              "    return g_strdup(\"\");\n"
              "}\n"
              "\n"
              "static void bar()\n"
              "{\n"
              "    int *p = g_malloc(16);\n"
              "    free_pointers(1, p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void func19() {
        // Ticket #2056
        check("bool a(int *p) {\n"
              "    return p;\n"
              "}\n"
              "\n"
              "void b() {\n"
              "    int *p = malloc(16);\n"
              "    if (!a(p)) return;\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("bool a(int *p) {\n"
              "    return p;\n"
              "}\n"
              "\n"
              "void b() {\n"
              "    int *p = g_malloc(16);\n"
              "    if (!a(p)) return;\n"
              "    g_free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void func20() {
        // Ticket #2182 - false positive when there is unused class.
        // If the unused class is removed the false positive goes away.
        // [test.cpp:12]: (error) Deallocating a deallocated pointer: p
        check("class test {\n"
              "    void f();\n"
              "};\n"
              "void test::f() { }\n"
              "\n"
              "void b(int i) {\n"
              "    char *p = new char[10];\n"
              "    if (i) {\n"
              "        delete [] p;\n"
              "        exit(0);\n"
              "    }\n"
              "    delete [] p;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // False positive in classmember
        // same code as above but the implementation is used in the
        // class member function
        check("class test {\n"
              "    void f(int i);\n"
              "};\n"
              "void test::f(int i) {\n"
              "    char *p = new char[10];\n"
              "    if (i) {\n"
              "        delete [] p;\n"
              "        exit(0);\n"
              "    }\n"
              "    delete [] p;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    //# Ticket 2569
    void func21() {
        // checking for lstat function:
        // ----------------------------
        check("void foo ()\n"
              "{\n"
              "    struct stat CFileAttr;\n"
              "    char *cpFile = new char [13];\n"
              "    strcpy (cpFile, \"testfile.txt\");\n"
              "    if (lstat (cpFile, &CFileAttr) != 0)\n"
              "    {\n"
              "        return;\n"
              "    }\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (error) Memory leak: cpFile\n", errout.str());

        check("void foo ()\n"
              "{\n"
              "    struct stat CFileAttr;\n"
              "    char *cpFile = new char [13];\n"
              "    strcpy (cpFile, \"testfile.txt\");\n"
              "    if (lstat (cpFile, &CFileAttr) != 0)\n"
              "    {\n"
              "        delete [] cpFile;\n"
              "        return;\n"
              "    }\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11]: (error) Memory leak: cpFile\n", errout.str());

        check("void foo ()\n"
              "{\n"
              "    struct stat CFileAttr;\n"
              "    char *cpFile = new char [13];\n"
              "    strcpy (cpFile, \"testfile.txt\");\n"
              "    if (lstat (cpFile, &CFileAttr) != 0)\n"
              "    {\n"
              "        delete [] cpFile;\n"
              "        return;\n"
              "    }\n"
              "    delete [] cpFile;\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("", errout.str());


        /// checking for stat function:
        // ----------------------------
        check("void foo ()\n"
              "{\n"
              "    struct stat CFileAttr;\n"
              "    char *cpFile = new char [13];\n"
              "    strcpy (cpFile, \"testfile.txt\");\n"
              "    if ( stat (cpFile, &CFileAttr) != 0)\n"
              "    {\n"
              "        return;\n"
              "    }\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (error) Memory leak: cpFile\n", errout.str());

        check("void foo ()\n"
              "{\n"
              "    struct stat CFileAttr;\n"
              "    char *cpFile = new char [13];\n"
              "    strcpy (cpFile, \"testfile.txt\");\n"
              "    if ( stat (cpFile, &CFileAttr) != 0)\n"
              "    {\n"
              "        delete [] cpFile;\n"
              "        return;\n"
              "    }\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11]: (error) Memory leak: cpFile\n", errout.str());

        check("void foo ()\n"
              "{\n"
              "    struct stat CFileAttr;\n"
              "    char *cpFile = new char [13];\n"
              "    strcpy (cpFile, \"testfile.txt\");\n"
              "    if ( stat (cpFile, &CFileAttr) != 0)\n"
              "    {\n"
              "        delete [] cpFile;\n"
              "        return;\n"
              "    }\n"
              "    delete [] cpFile;\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // checking for access function
        // http://www.gnu.org/s/libc/manual/html_node/Testing-File-Access.html
        // --------------------------------------------------------------------

        check("void foo ()\n"
              "{\n"
              "    struct stat CFileAttr;\n"
              "    char *cpFile = new char [13];\n"
              "    strcpy (cpFile, \"testfile.txt\");\n"
              "    if ( access (cpFile, R_OK) != 0)\n"
              "    {\n"
              "        return;\n"
              "    }\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (error) Memory leak: cpFile\n", errout.str());

        check("void foo ()\n"
              "{\n"
              "    struct stat CFileAttr;\n"
              "    char *cpFile = new char [13];\n"
              "    strcpy (cpFile, \"testfile.txt\");\n"
              "    if (access (cpFile, R_OK) != 0)\n"
              "    {\n"
              "        delete [] cpFile;\n"
              "        return;\n"
              "    }\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11]: (error) Memory leak: cpFile\n", errout.str());

        check("void foo ()\n"
              "{\n"
              "    struct stat CFileAttr;\n"
              "    char *cpFile = new char [13];\n"
              "    strcpy (cpFile, \"testfile.txt\");\n"
              "    if (access (cpFile, R_OK) != 0)\n"
              "    {\n"
              "        delete [] cpFile;\n"
              "        return;\n"
              "    }\n"
              "    delete [] cpFile;\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("", errout.str());


        // checking for chdir function
        // http://home.fhtw-berlin.de/~junghans/cref/MAN/chdir.htm
        // --------------------------------------------------------

        check("void foo()\n"
              "{\n"
              "    char * cpDir = new char [7];\n"
              "    strcpy (cpDir, \"/home/\");\n"
              "    if (chdir (cpDir) != 0)\n"
              "    {\n"
              "        return;\n"
              "    }\n"
              "    delete [] cpDir;\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Memory leak: cpDir\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    char * cpDir = new char [7];\n"
              "    strcpy (cpDir, \"/home/\");\n"
              "    if (chdir (cpDir) != 0)\n"
              "    {\n"
              "        delete [] cpDir;\n"
              "        return;\n"
              "    }\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (error) Memory leak: cpDir\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    char * cpDir = new char [7];\n"
              "    strcpy (cpDir, \"/home/\");\n"
              "    if (chdir (cpDir) != 0)\n"
              "    {\n"
              "        delete [] cpDir;\n"
              "        return;\n"
              "    }\n"
              "    delete [] cpDir;\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // checking for chmod function
        // http://publib.boulder.ibm.com/infocenter/zos/v1r10/index.jsp?topic=/com.ibm.zos.r10.bpxbd00/rtchm.htm
        // ------------------------------------------------------------------------------------------------------

        check("void foo()\n"
              "{\n"
              "    char * cpDir = new char [7];\n"
              "    strcpy (cpDir, \"/home/\");\n"
              "    if (chmod(cpDir, S_IRWXU|S_IRWXG) != 0)\n"
              "    {\n"
              "        return;\n"
              "    }\n"
              "    delete [] cpDir;\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Memory leak: cpDir\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    char * cpDir = new char [7];\n"
              "    strcpy (cpDir, \"/home/\");\n"
              "    if (chmod(cpDir, S_IRWXU|S_IRWXG) != 0)\n"
              "    {\n"
              "        delete [] cpDir;\n"
              "        return;\n"
              "    }\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (error) Memory leak: cpDir\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    char * cpDir = new char [7];\n"
              "    strcpy (cpDir, \"/home/\");\n"
              "    if (chmod(cpDir, S_IRWXU|S_IRWXG) != 0)\n"
              "    {\n"
              "        delete [] cpDir;\n"
              "        return;\n"
              "    }\n"
              "    delete [] cpDir;\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("", errout.str());


        // checking for chown function
        // http://publib.boulder.ibm.com/infocenter/zos/v1r10/index.jsp?topic=/com.ibm.zos.r10.bpxbd00/rtchm.htm
        // ------------------------------------------------------------------------------------------------------

        check("void foo()\n"
              "{\n"
              "    char * cpDir = new char [7];\n"
              "    strcpy (cpDir, \"/home/\");\n"
              "    if (chown(cpDir, 25, 0) != 0)\n"
              "    {\n"
              "        return;\n"
              "    }\n"
              "    delete [] cpDir;\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Memory leak: cpDir\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    char * cpDir = new char [7];\n"
              "    strcpy (cpDir, \"/home/\");\n"
              "    if (chown(cpDir, 25, 0) != 0)\n"
              "    {\n"
              "        delete [] cpDir;\n"
              "        return;\n"
              "    }\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (error) Memory leak: cpDir\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    char * cpDir = new char [7];\n"
              "    strcpy (cpDir, \"/home/\");\n"
              "    if (chown(cpDir, 25, 0) != 0)\n"
              "    {\n"
              "        delete [] cpDir;\n"
              "        return;\n"
              "    }\n"
              "    delete [] cpDir;\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // checking perror
        //http://www.cplusplus.com/reference/clibrary/cstdio/perror/
        // ---------------------------------------------------------

        check("void foo()\n"
              "{\n"
              "    char *cBuf = new char[11];\n"
              "    sprintf(cBuf,\"%s\",\"testtest..\");\n"
              "    perror (cBuf);\n"
              "    delete [] cBuf;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    char *cBuf = new char[11];\n"
              "    sprintf(cBuf,\"%s\",\"testtest..\");\n"
              "    perror (cBuf);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Memory leak: cBuf\n", errout.str());
    }

    // # 2668
    void func22() {
        check("void  foo()\n"
              "{\n"
              "    char * cpFile;\n"
              "    cpFile = new char [13];\n"
              "    strcpy (cpFile, \"testfile.txt\");\n"
              "    if(freopen(cpFile,\"w\",stdout)==0)\n"
              "    {\n"
              "        return;\n"
              "    }\n"
              "    delete [] cpFile;\n"
              "    fclose (stdout);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Memory leak: cpFile\n", errout.str());

        check("void  foo()\n"
              "{\n"
              "    char * cpFile;\n"
              "    cpFile = new char [13];\n"
              "    strcpy (cpFile, \"testfile.txt\");\n"
              "    if(freopen(cpFile,\"w\",stdout)==0)\n"
              "    {\n"
              "        delete [] cpFile;\n"
              "        return;\n"
              "    }\n"
              "    fclose (stdout);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:12]: (error) Memory leak: cpFile\n", errout.str());

        check("void  foo()\n"
              "{\n"
              "    char * cpFile;\n"
              "    cpFile = new char [13];\n"
              "    strcpy (cpFile, \"testfile.txt\");\n"
              "    if(freopen(cpFile,\"w\",stdout)==0)\n"
              "    {\n"
              "        delete [] cpFile;\n"
              "        return;\n"
              "    }\n"
              "    delete [] cpFile;\n"
              "    fclose (stdout);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    // # 2667
    void func23() {

        // check open() function
        // ----------------------
        check("int * foo()\n"
              "{\n"
              "    char * cpFile = new char [13];\n"
              "    strcpy (cpFile, \"testfile.txt\");\n"
              "    int file=open(cpFile,O_RDONLY);\n"
              "    if(file < -1)\n"
              "    {\n"
              "        return file;\n"
              "    }\n"
              "    delete [] cpFile;\n"
              "    return file;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Memory leak: cpFile\n", errout.str());

        check("int * foo()\n"
              "{\n"
              "    char * cpFile = new char [13];\n"
              "    strcpy (cpFile, \"testfile.txt\");\n"
              "    int file=open(cpFile,O_RDONLY);\n"
              "    if(file < -1)\n"
              "    {\n"
              "        delete [] cpFile;\n"
              "        return file;\n"
              "    }\n"
              "    return file;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11]: (error) Memory leak: cpFile\n", errout.str());

        check("int * foo()\n"
              "{\n"
              "    char * cpFile = new char [13];\n"
              "    strcpy (cpFile, \"testfile.txt\");\n"
              "    int file=open(cpFile,O_RDONLY);\n"
              "    if(file < -1)\n"
              "    {\n"
              "        delete [] cpFile;\n"
              "        return file;\n"
              "    }\n"
              "    delete [] cpFile;\n"
              "    return file;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // check for _open, _wopen
        // http://msdn.microsoft.com/en-us/library/z0kc8e3z(VS.80).aspx
        // -------------------------------------------------------------
        check("int * foo()\n"
              "{\n"
              "    char * cpFile = new char [13];\n"
              "    strcpy (cpFile, \"testfile.txt\");\n"
              "    int file=_open(cpFile,_O_RDONLY);\n"
              "    if(file == -1)\n"
              "    {\n"
              "        return file;\n"
              "    }\n"
              "    delete [] cpFile;\n"
              "    return file;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Memory leak: cpFile\n", errout.str());

        check("int * foo()\n"
              "{\n"
              "    char * cpFile = new char [13];\n"
              "    strcpy (cpFile, \"testfile.txt\");\n"
              "    int file=_open(cpFile,_O_RDONLY);\n"
              "    if(file == -1)\n"
              "    {\n"
              "        delete [] cpFile;\n"
              "        return file;\n"
              "    }\n"
              "    return file;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11]: (error) Memory leak: cpFile\n", errout.str());

        check("int * foo()\n"
              "{\n"
              "    char * cpFile = new char [13];\n"
              "    strcpy (cpFile, \"testfile.txt\");\n"
              "    int file=_open(cpFile,_O_RDONLY);\n"
              "    if(file == -1)\n"
              "    {\n"
              "        delete [] cpFile;\n"
              "        return file;\n"
              "    }\n"
              "    delete [] cpFile;\n"
              "    return file;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    // #2705
    void func24() {
        check("void f(void)\n"
              "{\n"
              "  std::string *x = new std::string;\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (error) Memory leak: x\n","", errout.str());

        check("void f(void)\n"
              "{\n"
              "  std::string *x = new std::string;\n"
              "  delete x;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void func25() { // ticket #2904
        check("class Fred { };\n"
              "void f(void)\n"
              "{\n"
              "  Fred *f = new Fred();\n"
              "  delete f;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class Fred { };\n"
              "void f(void)\n"
              "{\n"
              "  Fred *f = new Fred();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: f\n", errout.str());

        check("class Fred { void foo(){ } };\n"
              "void f(void)\n"
              "{\n"
              "  Fred *f = new Fred();\n"
              "  delete f;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class Fred { void foo(){ } };\n"
              "void f(void)\n"
              "{\n"
              "  Fred *f = new Fred();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: f\n", errout.str());
    }

    void func26() { // ticket #3444
        // technically there is a leak here. However warning is not wanted
        check("void f() {\n"
              "    char *p = strdup(\"A=B\");\n"
              "    putenv(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void func27() { // ticket #2773
        check("void f(FRED *pData)\n"
              "{\n"
              "    pData =(FRED*)malloc( 100 );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: pData\n", errout.str());

        check("void f(int *pData)\n"
              "{\n"
              "    pData =(int*)malloc( 100 );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: pData\n", errout.str());
    }

    void func28() { // ticket #3236
        check("void f()\n"
              "{\n"
              "    my_struct *p = malloc(42);\n"
              "    p->a + p->b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: p\n", errout.str());
    }

    void allocfunc1() {
        check("static char *a()\n"
              "{\n"
              "    return new char[100];\n"
              "}\n"
              "static void b()\n"
              "{\n"
              "    char *p = a();\n"
              "}");
        ASSERT_EQUALS(std::string("[test.cpp:8]: (error) Memory leak: p\n"), errout.str());

        check("FILE *a()\n"
              "{\n"
              "    return fopen(\"test.txt\",\"w\");\n"
              "}\n"
              "static void b()\n"
              "{\n"
              "    FILE *p = a();\n"
              "}");
        ASSERT_EQUALS(std::string("[test.cpp:8]: (error) Resource leak: p\n"), errout.str());

        check("char *a()\n"
              "{\n"
              "    return malloc(10);\n"
              "}\n"
              "static void b()\n"
              "{\n"
              "    char *p = a();\n"
              "}");
        ASSERT_EQUALS(std::string("[test.cpp:8]: (error) Memory leak: p\n"), errout.str());
    }

    void allocfunc2() {
        check("static char *a(int size)\n"
              "{\n"
              "    return new char[size];\n"
              "}\n"
              "static void b()\n"
              "{\n"
              "    int len = 100;\n"
              "    char *p = a(len);\n"
              "    delete [] p;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("char *a(char *a)\n"
              "{\n"
              "    return realloc(a, 10);\n"
              "}\n"
              "static void b()\n"
              "{\n"
              "    char *p = a(0);\n"
              "    char *q = a(p);\n"
              "    if (q)\n"
              "        free(q);\n"
              "    else\n"
              "        free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("char *a()\n"
              "{\n"
              "    return malloc(10);\n"
              "}\n"
              "static void b()\n"
              "{\n"
              "    char *p = a();\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("gchar *a()\n"
              "{\n"
              "    return g_malloc(10);\n"
              "}\n"
              "static void b()\n"
              "{\n"
              "    gchar *p = a();\n"
              "    g_free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void allocfunc3() {
        check("static char *a()\n"
              "{\n"
              "    char *data = malloc(10);;"
              "    return data;\n"
              "}\n"
              "static void b()\n"
              "{\n"
              "    char *p = a();\n"
              "}");
        ASSERT_EQUALS(std::string("[test.cpp:8]: (error) Memory leak: p\n"), errout.str());
    }

    void allocfunc4() {
        check("char* foo()\n"
              "{\n"
              "   char *str = NULL;\n"
              "   str = realloc( str, 20 );\n"
              "   return str;\n"
              "}\n"
              "\n"
              "void bar()\n"
              "{\n"
              "   char *p = foo();\n"
              "}");
        ASSERT_EQUALS(std::string("[test.cpp:11]: (error) Memory leak: p\n"), errout.str());

        check("char* foo()\n"
              "{\n"
              "   char *str = NULL;\n"
              "   str = realloc( str, 20 );\n"
              "   return str;\n"
              "}\n"
              "\n"
              "void bar()\n"
              "{\n"
              "   char *p = foo();\n"
              "   delete p;\n"
              "}");
        ASSERT_EQUALS(std::string("[test.cpp:11]: (error) Mismatching allocation and deallocation: p\n"), errout.str());
    }

    void allocfunc5() {
        check("void foo(char **str)\n"
              "{\n"
              "   *str = malloc(20);\n"
              "}\n"
              "\n"
              "void bar()\n"
              "{\n"
              "   char *p;\n"
              "   foo(&p);\n"
              "}");
        ASSERT_EQUALS(std::string("[test.cpp:10]: (error) Memory leak: p\n"), errout.str());

        check("void foo(char **str)\n"
              "{\n"
              "   *str = malloc(20);\n"
              "}\n"
              "\n"
              "void bar()\n"
              "{\n"
              "   char *p;\n"
              "   foo(&p);\n"
              "   delete p;\n"
              "}");
        ASSERT_EQUALS(std::string("[test.cpp:10]: (error) Mismatching allocation and deallocation: p\n"), errout.str());

        check("void foo(char **q, char **str)\n"
              "{\n"
              "   *str = malloc(20);\n"
              "}\n"
              "\n"
              "void bar()\n"
              "{\n"
              "   char *p;\n"
              "   char *q;\n"
              "   foo(&q, &p);\n"
              "}");
        ASSERT_EQUALS(std::string("[test.cpp:11]: (error) Memory leak: p\n"), errout.str());

        check("void foo(char **str)\n"
              "{\n"
              "   char *a = malloc(20);\n"
              "   *str = a;\n"
              "}\n"
              "\n"
              "void bar()\n"
              "{\n"
              "   char *p;\n"
              "   foo(&p);\n"
              "}");
        TODO_ASSERT_EQUALS(std::string("[test.cpp:11]: (error) Memory leak: p\n"),
                           "", errout.str());

        check("void foo(char **str)\n"
              "{\n"
              "    free(*str);\n"
              "    *str = malloc(20);\n"
              "}\n"
              "\n"
              "void bar()\n"
              "{\n"
              "   char *tmp = malloc(10);\n"
              "   foo(&tmp);\n"
              "   foo(&tmp);\n"
              "   free(tmp);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo(gchar **str)\n"
              "{\n"
              "    g_free(*str);\n"
              "    *str = g_malloc(20);\n"
              "}\n"
              "\n"
              "void bar()\n"
              "{\n"
              "   gchar *tmp = g_malloc(10);\n"
              "   foo(&tmp);\n"
              "   foo(&tmp);\n"
              "   g_free(tmp);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        //#ticket 1789: getcode other function:
        check("void foo(char **str)\n"
              "{\n"
              "    if (*str == NULL)\n"
              "        *str = malloc(20)\n;"
              "    else\n"
              "        *str = realloc(*str, 20)\n;"
              "}\n"
              "\n"
              "void bar()\n"
              "{\n"
              "   char *tmp = malloc(10);\n"
              "   foo(&tmp);\n"
              "   foo(&tmp);\n"
              "   free(tmp);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int alloc(char **str) {\n"
              "   *str = malloc(20);\n"
              "   if (condition) { free(str); return -123; }\n"
              "   return 0;\n"
              "}\n"
              "\n"
              "void bar()\n"
              "{\n"
              "   char *p;\n"
              "   if ((ret = alloc(&p)) != 0) return;\n"
              "   free(p);\n"
              "}");
        ASSERT_EQUALS(std::string(""), errout.str());
    }


    void allocfunc6() {
        check("static FILE* data()\n"
              "{\n"
              "    return fopen(\"data.txt\",\"rt\");\n"
              "}\n"
              "\n"
              "static void foo()\n"
              "{\n"
              "    char* expr;\n"
              "    func(&expr);\n"
              "\n"
              "    FILE *f = data();\n"
              "    fclose(f);\n"
              "\n"
              "    free(expr);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("static FILE* data()\n"
              "{\n"
              "    return fopen(\"data.txt\",\"rt\");\n"
              "}\n"
              "\n"
              "static void foo()\n"
              "{\n"
              "    gchar* expr;\n"
              "    func(&expr);\n"
              "\n"
              "    FILE *f = data();\n"
              "    fclose(f);\n"
              "\n"
              "    g_free(expr);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void allocfunc7() {
        // Ticket #2374 - no false positive
        check("char *data()\n"
              "{\n"
              "    char *s = malloc(100);\n"
              "    strings[0] = s;\n"
              "    return s;\n"
              "}\n"
              "\n"
              "static void foo()\n"
              "{\n"
              "    char* s = data();\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void allocfunc8() {
        // Ticket #2213 - calling alloc function twice
        check("FILE *foo() {\n"
              "    return fopen(a,b);\n"
              "}\n"
              "\n"
              "void bar() {\n"
              "    FILE *f = foo();\n"
              "    f = foo();\n"
              "    fclose(f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Resource leak: f\n", errout.str());
    }

    void allocfunc9() {
        // Ticket #2673 - address is taken in alloc func
        check("char *addstring(const char *s) {\n"
              "    char *ret = strdup(s);\n"
              "    strings.push_back(ret);"
              "    return ret;\n"
              "}\n"
              "\n"
              "void foo() {\n"
              "    char *s = addstring(\"abc\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void allocfunc10() {
        // Ticket #2921 - static pointer
        check("char *getstr() {\n"
              "    static char *ret = malloc(100);\n"
              "    return ret;\n"
              "}\n"
              "\n"
              "void foo() {\n"
              "    char *s = getstr();\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void allocfunc11() { // ticket #3809 - false positive
        check("void f (double  * & data_val)  {\n"
              "  data_val = malloc(0x100);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void allocfunc12() { // #3660: allocating and returning non-local pointer => not allocfunc
        check("char *p;\n" // global pointer
              "char *a()  {\n"
              "  if (!p) p = malloc(10);\n"
              "  return p;\n"
              "}\n"
              "void b() {\n"
              "    char *x = a();\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void allocfunc13() { // #4494: class function
        check("namespace n {\n"
              "    char *a() { return malloc(100); }\n"
              "}\n"
              "void b() {\n"
              "    char *x = n::a();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Memory leak: x\n", errout.str());

        check("class C {\n"  // ¤4540
              "    char *a() { return malloc(100); }\n"
              "}\n"
              "void b() {\n"
              "    C c;"
              "    char *x = c.a();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Memory leak: x\n", errout.str());

        check("class Pool{\n"
              "    int* GetNewObj()\n"
              "    {\n"
              "        return new int;\n"
              "    }\n"
              "};\n"
              "void foo(){\n"
              "    Pool pool;\n"
              "    int* a = pool.GetNewObj();\n"
              "    int* b = GetNewObj();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11]: (error) Memory leak: a\n", errout.str());
    }

    void allocfunc14() { // use pointer before returning it
        check("static struct ABC * newabc() {\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    init_abc(&abc->a);\n" // <- might take address
              "    return abc;\n"
              "}\n"
              "\n"
              "static void f() {\n"
              "    struct ABC *abc = newabc();\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void throw1() {
        check("void foo()\n"
              "{\n"
              "    char *str = new char[10];\n"
              "    if ( ! abc )\n"
              "        throw 123;\n"
              "    delete [] str;\n"
              "}");

        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: str\n", errout.str());
    }

    void throw2() {
        check("void foo()\n"
              "{\n"
              "    char *str = 0;\n"
              "    try\n"
              "    {\n"
              "        str = new char[100];\n"
              "        if ( somecondition )\n"
              "            throw exception;\n"
              "        delete [] str;\n"
              "    }\n"
              "    catch ( ... )\n"
              "    {\n"
              "        delete [] str;\n"
              "    }\n"
              "}");

        ASSERT_EQUALS("", errout.str());
    }




    void linux_list_1() {
        check("struct AB\n"
              "{\n"
              "    int a;\n"
              "    int b;\n"
              "};\n"
              "void foo()\n"
              "{\n"
              "    struct AB *ab = new AB;\n"
              "    func(&ab->a);\n"
              "}");

        ASSERT_EQUALS("", errout.str());
    }

    void linux_list_2() { // #5993
        check("void foo() {\n"
              "    struct AB *ab = malloc(sizeof(struct AB));\n"
              "    list_add_tail(&(ab->list));\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }



    void sizeof1() {
        check("void f()\n"
              "{\n"
              "    struct s_t s1;\n"
              "    struct s_t cont *p = &s1;\n"
              "    struct s_t *s2;\n"
              "\n"
              "    memset(p, 0, sizeof(*p));\n"
              "\n"
              "    s2 = (struct s_t *) malloc(sizeof(*s2));\n"
              "\n"
              "    if (s2->value != 0)\n"
              "        return;\n"
              "\n"
              "    free(s2);\n"
              "\n"
              "    return;\n"
              "}");

        ASSERT_EQUALS("[test.cpp:12]: (error) Memory leak: s2\n", errout.str());
        check("void f()\n"
              "{\n"
              "    struct s_t s1;\n"
              "    struct s_t cont *p = &s1;\n"
              "    struct s_t *s2;\n"
              "\n"
              "    memset(p, 0, sizeof(*p));\n"
              "\n"
              "    s2 = (struct s_t *) g_malloc(sizeof(*s2));\n"
              "\n"
              "    if (s2->value != 0)\n"
              "        return;\n"
              "\n"
              "    g_free(s2);\n"
              "\n"
              "    return;\n"
              "}");

        ASSERT_EQUALS("[test.cpp:12]: (error) Memory leak: s2\n", errout.str());
    }


    void realloc1() {
        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    a = realloc(a, 100);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Common realloc mistake: \'a\' nulled but not freed upon failure\n"
                      "[test.cpp:5]: (error) Memory leak: a\n", errout.str());
    }

    void realloc2() {
        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    a = (char *)realloc(a, 100);\n"
              "    free(a);\n"
              "}");

        ASSERT_EQUALS("[test.cpp:4]: (error) Common realloc mistake: \'a\' nulled but not freed upon failure\n", errout.str());
    }

    void realloc3() {
        check("void foo()\n"
              "{\n"
              "    char *a = 0;\n"
              "    if ((a = realloc(a, 100)) == NULL)\n"
              "        return;\n"
              "    free(a);\n"
              "}");

        ASSERT_EQUALS("", errout.str());
    }

    void realloc4() {
        check("void foo()\n"
              "{\n"
              "    static char *a = 0;\n"
              "    if ((a = realloc(a, 100)) == NULL)\n"
              "        return;\n"
              "    free(a);\n"
              "}");

        TODO_ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: a\n",

                           "[test.cpp:4]: (error) Common realloc mistake: \'a\' nulled but not freed upon failure\n",
                           errout.str());
    }

    void realloc5() {
        check("void foo()\n"
              "{\n"
              "    char *buf;\n"
              "    char *new_buf;\n"
              "    buf = calloc( 10 );\n"
              "    new_buf = realloc ( buf, 20);\n"
              "    if ( !new_buf )\n"
              "        free(buf);\n"
              "    else\n"
              "        free(new_buf);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void realloc6() {
        ASSERT_EQUALS(";;realloc;;", getcode("char *buf; buf=realloc(buf,100);", "buf"));
        ASSERT_EQUALS(";;alloc;", getcode("char *buf; buf=realloc(0,100);", "buf"));
    }

    void realloc7() {
        check("bool foo(size_t nLen, char* pData)\n"
              "{\n"
              "    pData = (char*) realloc(pData, sizeof(char) + (nLen + 1)*sizeof(char));\n"
              "    if ( pData == NULL )\n"
              "    {\n"
              "        return false;\n"
              "    }\n"
              "    free(pData);\n"
              "    return true;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void realloc8() {
        check("void foo()\n"
              "{\n"
              "    char *origBuf = m_buf;\n"
              "    m_buf = (char *) realloc (m_buf, m_capacity + growBy);\n"
              "    if (!m_buf) {\n"
              "        m_buf = origBuf;\n"
              "    }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void realloc9() {
        check("void foo()\n"
              "{\n"
              "    x = realloc(x,100);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void realloc10() {
        check("void foo() {\n"
              "    char *pa, *pb;\n"
              "    pa = pb = malloc(10);\n"
              "    pa = realloc(pa, 20);"
              "    exit();\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void realloc11() {
        check("void foo() {\n"
              "    char *p;\n"
              "    p = realloc(p, size);\n"
              "    if (!p)\n"
              "        error();\n"
              "    usep(p);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void realloc12() {
        check("void foo(int x)\n"
              "{\n"
              "    char *a = 0;\n"
              "    if ((a = realloc(a, x + 100)) == NULL)\n"
              "        return;\n"
              "    free(a);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void realloc13() {
        check("void foo()\n"
              "{\n"
              "    char **str;\n"
              "    *str = realloc(*str,100);\n"
              "    free (*str);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Common realloc mistake: \'str\' nulled but not freed upon failure\n", errout.str());
    }

    void realloc14() {
        check("void foo() {\n"
              "    char *p;\n"
              "    p = realloc(p, size + 1);\n"
              "    if (!p)\n"
              "        error();\n"
              "    usep(p);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void realloc15() {
        check("bool foo() {\n"
              "    char ** m_options;\n"
              "    m_options = (char**)realloc( m_options, 2 * sizeof(char*));\n"
              "    if( m_options == NULL )\n"
              "        return false;\n"
              "    return true;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) Common realloc mistake: \'m_options\' nulled but not freed upon failure\n"
                      "[test.cpp:6]: (error) Memory leak: m_options\n", errout.str());
    }

    void realloc16() {
        check("void f(char *zLine) {\n"
              "  zLine = realloc(zLine, 42);\n"
              "  if (zLine) {\n"
              "    free(zLine);\n"
              "  }\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void assign1() {
        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    a = 0;\n"
              "    free(a);\n"
              "}");

        ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: a\n", errout.str());
        check("void foo()\n"
              "{\n"
              "    gchar *a = (gchar *)g_malloc(10);\n"
              "    a = 0;\n"
              "    g_free(a);\n"
              "}");

        ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: a\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    char *p = a;\n"
              "    free(p);\n"
              "}");

        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    gchar *a = (gchar *)g_malloc(10);\n"
              "    gchar *p = a;\n"
              "    g_free(p);\n"
              "}");

        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    char *p = a + 1;\n"
              "    free(p);\n"
              "}");

        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    gchar *a = (gchar *)g_malloc(10);\n"
              "    gchar *p = a + 1;\n"
              "    g_free(p);\n"
              "}");

        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    a += 10;\n"
              "    free(a - 10);\n"
              "}");

        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    gchar *a = (gchar *)g_malloc(10);\n"
              "    a += 10;\n"
              "    g_free(a - 10);\n"
              "}");

        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    char *a = (char *)malloc(10);\n"
              "    a = (void *)a + 10;\n"
              "    free(a - 10);\n"
              "}");

        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    gchar *a = (gchar *)g_malloc(10);\n"
              "    a = (void *)a + 10;\n"
              "    g_free(a - 10);\n"
              "}");

        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    char *a = new char[100];\n"
              "    list += a;\n"
              "}");

        ASSERT_EQUALS("", errout.str());
    }

    void assign2() {
        // #2806 - FP when there is redundant assignment
        check("void foo() {\n"
              "   gchar *bar;\n"
              "   bar = g_strdup(fuba);\n"
              "   bar = g_strstrip(bar);\n"
              "   g_free(bar);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varid() {
        check("void foo()\n"
              "{\n"
              "    char *p = malloc(100);\n"
              "    {\n"
              "        char *p = 0;\n"
              "        delete p;\n"
              "    }\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    gchar *p = g_malloc(100);\n"
              "    {\n"
              "        gchar *p = 0;\n"
              "        delete p;\n"
              "    }\n"
              "    g_free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void cast1() {
        check("void foo()\n"
              "{\n"
              "    char *a = reinterpret_cast<char *>(malloc(10));\n"
              "}");

        ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: a\n", errout.str());
        check("void foo()\n"
              "{\n"
              "    char *a = reinterpret_cast<char *>(g_malloc(10));\n"
              "}");

        ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: a\n", errout.str());
    }



    void dealloc_use() {
        // It is ok to take the address..
        check("void f()\n"
              "{\n"
              "    char *s = new char[100];\n"
              "    delete [] s;\n"
              "    p = s;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    char *s = new char[100];\n"
              "    delete [] s;\n"
              "    foo(s);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    char *s = new char[100];\n"
              "    delete [] s;\n"
              "    printf(\"%p\\n\", s);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // The pointer to the pointer is valid..
        check("void f()\n"
              "{\n"
              "    char *str;\n"
              "    free(str);\n"
              "    foo(&str);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void f()\n"
              "{\n"
              "    gchar *str;\n"
              "    g_free(str);\n"
              "    foo(&str);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    char *str = 0;\n"
              "    free(str);\n"
              "    f1(&str);\n"
              "    f2(str);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    gchar *str = 0;\n"
              "    g_free(str);\n"
              "    f1(&str);\n"
              "    f2(str);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Dereferencing the freed pointer is not ok..
        check("void foo()\n"
              "{\n"
              "    char *str = malloc(10);\n"
              "    free(str);\n"
              "    char c = *str;\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (error) Dereferencing 'str' after it is deallocated / released\n",
                           "", errout.str());
        check("void foo()\n"
              "{\n"
              "    gchar *str = g_malloc(10);\n"
              "    g_free(str);\n"
              "    gchar c = *str;\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (error) Dereferencing 'str' after it is deallocated / released\n",
                           "", errout.str());

        check("void foo()\n"
              "{\n"
              "    char *str = malloc(10);\n"
              "    free(str);\n"
              "    char c = str[10];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Dereferencing 'str' after it is deallocated / released\n", errout.str());
        check("void foo()\n"
              "{\n"
              "    gchar *str = g_malloc(10);\n"
              "    g_free(str);\n"
              "    gchar c = str[10];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Dereferencing 'str' after it is deallocated / released\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    char *str = malloc(10);\n"
              "    free(str);\n"
              "    str[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Dereferencing 'str' after it is deallocated / released\n", errout.str());
        check("void foo()\n"
              "{\n"
              "    gchar *str = g_malloc(10);\n"
              "    g_free(str);\n"
              "    str[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Dereferencing 'str' after it is deallocated / released\n", errout.str());

        check("void foo() {\n"
              "    char *str = malloc(10);\n"
              "    free(str);\n"
              "    strcpy(str, p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Dereferencing 'str' after it is deallocated / released\n", errout.str());
        check("void foo() {\n"
              "    gchar *str = g_malloc(10);\n"
              "    g_free(str);\n"
              "    g_strlcpy(str, p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Dereferencing 'str' after it is deallocated / released\n", errout.str());

        check("void foo(int x) {\n"
              "    char *str = malloc(10);\n"
              "    free(str);\n"
              "    assert(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo(int x) {\n"
              "    gchar *str = g_malloc(10);\n"
              "    g_free(str);\n"
              "    assert(x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void dealloc_use_2() {
        // #3041 - assigning pointer when it's used
        check("void f(char *s) {\n"
              "    free(s);\n"
              "    strcpy(a, s=b());\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void f(gchar *s) {\n"
              "    g_free(s);\n"
              "    g_strlcpy(a, s=b());\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void dealloc_use_3() {
        check("void foo()\n"
              "{\n"
              "    char *str = malloc(10);\n"
              "    realloc(str, 0);\n"
              "    str[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Dereferencing 'str' after it is deallocated / released\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    char *str = realloc(0, 10);\n"
              "    realloc(str, 0);\n"
              "    str[10] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Dereferencing 'str' after it is deallocated / released\n", errout.str());
    }

    void freefree1() {
        check("void foo()\n"
              "{\n"
              "    char *str = malloc(100);\n"
              "    free(str);\n"
              "    free(str);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Deallocating a deallocated pointer: str\n", errout.str());
        check("void foo()\n"
              "{\n"
              "    gchar *str = g_malloc(100);\n"
              "    g_free(str);\n"
              "    g_free(str);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Deallocating a deallocated pointer: str\n", errout.str());
    }

    void freefree2() {
        check("void foo()\n"
              "{\n"
              "    FILE *fd = fopen(\"test.txt\", \"wb\");\n"
              "    fprintf(fd, \"test\");\n"
              "    fclose(fd);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void freefree3() {
        check("void foo()\n"
              "{\n"
              "    char *p = malloc(10);\n"
              "    free(p);\n"
              "    bar(&p);\n"
              "    free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    gchar *p = g_malloc(10);\n"
              "    g_free(p);\n"
              "    bar(&p);\n"
              "    g_free(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void strcpy_result_assignment() {
        check("void foo()\n"
              "{\n"
              "    char *p1 = malloc(10);\n"
              "    char *p2 = strcpy(p1, \"a\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    char *p1 = g_malloc(10);\n"
              "    char *p2 = g_strlcpy(p1, \"a\");\n"
              "}");
        TODO_ASSERT_EQUALS("", "[test.cpp:5]: (error) Memory leak: p1\n", errout.str());
    }

    void strcat_result_assignment() {
        check("void foo()\n"
              "{\n"
              "    char *p = malloc(10);\n"
              "    p[0] = 0;\n"
              "    p = strcat( p, \"a\" );\n"
              "    free( p );\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    gchar *p = g_malloc(10);\n"
              "    p[0] = 0;\n"
              "    p = g_strcat( p, \"a\" );\n"
              "    g_free( p );\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }



    void all1() {
        check("void foo()\n"
              "{\n"
              "    Fred *f = new Fred;\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }



    void malloc_constant_1() {
        check("void foo()\n"
              "{\n"
              "    int *p = malloc(3);\n"
              "    free(p);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (error) The allocated size 3 is not a multiple of the underlying type's size.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    int *p = g_malloc(3);\n"
              "    g_free(p);\n"
              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (error) The allocated size 3 is not a multiple of the underlying type's size.\n", "", errout.str());
    }



    void unknownFunction1() {
        check("void foo()\n"
              "{\n"
              "    int *p = new int[100];\n"
              "    if (abc)\n"
              "    {\n"
              "        delete [] p;\n"
              "        ThrowException();\n"
              "    }\n"
              "    delete [] p;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void unknownFunction2() {
        check("void foo()\n"
              "{\n"
              "    int *p = new int[100];\n"
              "    if (abc)\n"
              "    {\n"
              "        delete [] p;\n"
              "        ThrowException();\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (error) Memory leak: p\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    int *p = new int[100];\n"
              "    p = g();\n"
              "    delete [] p;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: p\n", errout.str());
    }

    void unknownFunction4() {
        check("void foo()\n"
              "{\n"
              "    int *p = new int[100];\n"
              "    a();\n"
              "    if (b) return;\n"
              "    delete [] p;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: p\n", errout.str());
    }

    void unknownFunction5() {
        check("static void foo()\n"
              "{\n"
              "    char *p = NULL;\n"
              "\n"
              "    if( a )\n"
              "        p = malloc(100);\n"
              "\n"
              "    if( a )\n"
              "    {\n"
              "        FREENULL(p);\n"
              "        FREENULL();\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("static void foo()\n"
              "{\n"
              "    gchar *p = NULL;\n"
              "\n"
              "    if( a )\n"
              "        p = g_malloc(100);\n"
              "\n"
              "    if( a )\n"
              "    {\n"
              "        FREENULL(p);\n"
              "        FREENULL();\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void class1() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred()\n"
              "    {\n"
              "        int *p = new int[100];\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:7]: (error) Memory leak: p\n", errout.str());
    }

    void class2() {
        check("class Fred {\n"
              "public:\n"
              "    Fred() : rootNode(0) {}\n"
              "\n"
              "private:\n"
              "    struct Node {\n"
              "        Node(Node* p) {\n"
              "            parent = p;\n"
              "            if (parent) {\n"
              "                parent->children.append(this);\n"
              "            }\n"
              "        }\n"
              "\n"
              "        ~Node() {\n"
              "            qDeleteAll(children);\n"
              "        }\n"
              "\n"
              "        QList<Node*> children;\n"
              "    };\n"
              "\n"
              "    Node rootNode;\n"
              "\n"
              "    void f() {\n"
              "        Node* recordNode = new Node(&rootNode);\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }


    void autoptr1() {
        check("std::auto_ptr<int> foo()\n"
              "{\n"
              "    int *i = new int;\n"
              "    return std::auto_ptr<int>(i);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void if_with_and() {
        Settings settings;
        settings.experimental = true;

        check("void f()\n"
              "{\n"
              "  char *a = new char[10];\n"
              "  if (!a && b() )\n"
              "    return;\n"
              "\n"
              "  delete [] a;\n"
              "}\n", &settings);
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "  char *a = new char[10];\n"
              "  if (b() && !a )\n"
              "    return;\n"
              "\n"
              "  delete [] a;\n"
              "}\n", &settings);
        ASSERT_EQUALS("", errout.str());
    }

    void assign_pclose() {
        Settings settings;
        settings.standards.posix = true;

        check("void f() {\n"
              "  FILE *f = popen (\"test\", \"w\");\n"
              "  int a = pclose(f);\n"
              "}", &settings);
        ASSERT_EQUALS("", errout.str());
    }

    void exit2() {
        check("void f()\n"
              "{\n"
              "    char *out = new char[100];\n"
              "    exit(0);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    char *out = new char[100];\n"
              "    if( out ) {}\n"
              "    exit(0);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void exit4() {
        check("void f()\n"
              "{\n"
              "    char *p = malloc(100);\n"
              "    if (x)\n"
              "    {\n"
              "        exit(0);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Memory leak: p\n", errout.str());
    }

    void exit5() {
        check("void f()\n"
              "{\n"
              "    char *p = malloc(100);\n"
              "    if (p)\n"
              "    {\n"
              "        xyz();\n"
              "        exit(0);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void exit6() {
        check("int main(int argc, char *argv[]) {\n"
              "    FILE *sfile;\n"
              "    unsigned long line;\n"
              "    sfile = fopen(\"bar\", \"r\");\n"
              "    if (!sfile)\n"
              "        return 1;\n"
              "    for(line = 0; ; line++) {\n"
              "        if (argc > 3)\n"
              "            break;\n"
              "        exit(0);\n"
              "    }\n"
              "    fclose(sfile);\n"
              "    exit(0);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void exit7() {
        check("int a(int x) {\n"
              "    if (x == 0) {\n"
              "        exit(0);\n"
              "    }\n"
              "    return x + 2;\n"
              "}\n"
              "\n"
              "void b() {\n"
              "    char *p = malloc(100);\n"
              "    int i = a(123);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11]: (error) Memory leak: p\n", errout.str());
    }

    void noreturn() {
        check("void fatal_error()\n"
              "{ exit(1); }\n"
              "\n"
              "void f()\n"
              "{\n"
              "    char *p = malloc(100);\n"
              "    fatal_error();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void fatal_error()\n"     // #2440
              "{ }\n"
              "\n"
              "void f()\n"
              "{\n"
              "    char *p = malloc(100);\n"
              "    fatal_error();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Memory leak: p\n", errout.str());
    }

    void strndup_function() {
        check("void f() {\n"
              "    char *out = strndup(\"text\", 3);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Memory leak: out\n", errout.str());
    }

    void tmpfile_function() {
        check("void f()\n"
              "{\n"
              "    FILE *f = tmpfile();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Resource leak: f\n", errout.str());

        check("void f()\n"
              "{\n"
              "    FILE *f = tmpfile();\n"
              "    if (!f)\n"
              "        return;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Resource leak: f\n", errout.str());

        check("void f()\n"
              "{\n"
              "    FILE *f = tmpfile();\n"
              "    fclose(f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    FILE *f = tmpfile();\n"
              "    if (!f)\n"
              "        return;\n"
              "    fclose(f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("FILE *f()\n"
              "{\n"
              "    return tmpfile();\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void fcloseall_function() {
        check("void f()\n"
              "{\n"
              "    FILE *f = fopen(fname, str);\n"
              "    fcloseall();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    FILE *f = tmpfile();\n"
              "    fcloseall();\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void open_function() {
        Settings settings;
        settings.experimental = true;
        settings.standards.posix = true;

        check("void f(const char *path)\n"
              "{\n"
              "    int fd = open(path, O_RDONLY);\n"
              "}\n", &settings);
        ASSERT_EQUALS("[test.cpp:4]: (error) Resource leak: fd\n", errout.str());

        check("void f(const char *path)\n"
              "{\n"
              "    int fd = open(path, O_RDONLY);\n"
              "    if (fd == -1)\n"
              "       return;\n"
              "    close(fd);\n"
              "}\n", &settings);
        ASSERT_EQUALS("", errout.str());

        check("void f(const char *path)\n"
              "{\n"
              "    int fd = open(path, O_RDONLY);\n"
              "    if (fd < 0)\n"
              "       return;\n"
              "    close(fd);\n"
              "}\n", &settings);
        ASSERT_EQUALS("", errout.str());

        check("void f(const char *path)\n"
              "{\n"
              "    int fd = open(path, O_RDONLY);\n"
              "    if (-1 == fd)\n"
              "       return;\n"
              "    close(fd);\n"
              "}\n", &settings);
        ASSERT_EQUALS("", errout.str());
    }

    void creat_function() {
        Settings settings;
        settings.standards.posix = true;
        check("void f(const char *path)\n"
              "{\n"
              "    int fd = creat(path, S_IRWXU);\n"
              "}", &settings);
        ASSERT_EQUALS("[test.cpp:4]: (error) Resource leak: fd\n", errout.str());
    }

    void close_function() {
        Settings settings;
        settings.standards.posix = true;
        check("void f(const char *path)\n"
              "{\n"
              "    int fd = open(path, O_RDONLY);\n"
              "    close(fd);\n"
              "}", &settings);
        ASSERT_EQUALS("", errout.str());

        check("void f(const char *path)\n"
              "{\n"
              "    int fd = creat(path, S_IRWXU);\n"
              "    close(fd);\n"
              "}", &settings);
        ASSERT_EQUALS("", errout.str());

        check("void f(const char *path)\n"
              "{\n"
              "    int fd = creat(path, S_IRWXU);\n"
              "    if (close(fd) < 0) {\n"
              "        perror(\"close\");\n"
              "    }\n"
              "}", &settings);
        ASSERT_EQUALS("", errout.str());

        //#ticket 1401
        check("int myfunc()\n"
              "{\n"
              "  int handle;\n"
              "\n"
              "  handle = open(\"myfile\");\n"
              "  if (handle < 0) return 1;\n"
              "\n"
              "    while (some_condition())\n"
              "      if (some_other_condition())\n"
              "      {\n"
              "         close(handle);\n"
              "         return 3;\n"
              "      }\n"
              "  close(handle);\n"
              "}", &settings);
        ASSERT_EQUALS("", errout.str());

        //#ticket 1401
        check("int myfunc()\n"
              "{\n"
              "  int handle;\n"
              "\n"
              "  handle = open(\"myfile\", O_RDONLY);\n"
              "  if (handle < 0) return 1;\n"
              "\n"
              "    while (some_condition())\n"
              "      if (some_other_condition())\n"
              "      {\n"
              "         return 3;\n"
              "      }\n"
              "  close(handle);\n"
              "}", &settings);
        ASSERT_EQUALS("[test.cpp:11]: (error) Resource leak: handle\n", errout.str());
    }

    void fd_functions() {
        Settings settings;
        settings.standards.posix = true;
        check("void f(const char *path)\n"
              "{\n"
              "    int fd = open(path, O_RDONLY);\n"
              "    read(fd, buf, count);\n"
              "    readv(fd, iov, iovcnt);\n"
              "    readahead(fd, offset, count);\n"
              "    pread(fd, buf, count, offset);\n"
              "    write(fd, buf, count);\n"
              "    writev(fd, iov, iovcnt);\n"
              "    pwrite(fd, buf, count, offset);\n"
              "    ioctl(fd, request);\n"
              "    posix_fallocate(fd, offset, len);\n"
              "    posix_fadvise(fd, offset, len, advise);\n"
              "    fsync(fd);\n"
              "    fdatasync(fd);\n"
              "    sync_file_range(fd, offset, nbytes, flags);\n"
              "    lseek(fd, offset, whence);\n"
              "    fcntl(fd, cmd);\n"
              "    flock(fd, op);\n"
              "    lockf(fd, cmd, len);\n"
              "    ftruncate(fd, len);\n"
              "    fstat(fd, buf);\n"
              "    fchmod(fd, mode);\n"
              "}", &settings);
        ASSERT_EQUALS("[test.cpp:24]: (error) Resource leak: fd\n", errout.str());
    }

    void file_functions() {
        check("void f()\n"
              "{\n"
              "FILE *f = fopen(fname, str);\n"
              "feof(f);\n"
              "clearerr(in);\n"
              "ferror(in);\n"
              "fread(ptr, 10, 1, in);\n"
              "fwrite(ptr, 10, 1, in);\n"
              "fflush(in);\n"
              "setbuf(in, buf);\n"
              "setbuffer(in, buf, 100);\n"
              "setlinebuf(in);\n"
              "setvbuf(in, buf, _IOLBF, 0);\n"
              "fseek(in, 10, SEEK_SET);\n"
              "fseeko(in, 10, SEEK_SET);\n"
              "ftell(in);\n"
              "ftello(in);\n"
              "rewind(in);\n"
              "fsetpos(in, 0);\n"
              "fgetpos(in, 10);\n"
              "fprintf(in, \"text\\n\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:22]: (error) Resource leak: f\n", errout.str());
    }

    void getc_function() {
        {
            check("void f()\n"
                  "{"
                  "    int c;\n"
                  "    FILE *fin1a = fopen (\"FILE.txt\", \"r\");\n"
                  "    while ( (c = getc (fin1a)) != EOF)\n"
                  "    { }\n"
                  "}");
            ASSERT_EQUALS("[test.cpp:6]: (error) Resource leak: fin1a\n", errout.str());
        }

        {
            check("void f()\n"
                  "{\n"
                  "    int c;\n"
                  "    FILE *fin1b = fopen(\"FILE.txt\", \"r\");\n"
                  "    c = getc(fin1b);\n"
                  "}");
            ASSERT_EQUALS("[test.cpp:6]: (error) Resource leak: fin1b\n", errout.str());
        }
    }

    void pointer_to_pointer() {
        check("void f(char **data)\n"
              "{\n"
              "    char *c = new char[12];\n"
              "    *c = 0;\n"
              "    *data = c;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void dealloc_and_alloc_in_func() {
        check("char *f( const char *x )\n"
              "{\n"
              "  delete [] x;\n"
              "  return new char[10];\n"
              "}\n"
              "\n"
              "int main()\n"
              "{\n"
              "  char *a=0;\n"
              "  a = f( a );\n"
              "  a[0] = 1;\n"
              "  delete [] a;\n"
              "  return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void unknownSyntax1() {
        // I don't know what this syntax means so cppcheck should bail out
        check("void foo()\n"
              "{\n"
              "    void *sym = ( {\n"
              "                 void *__ptr = malloc(100);\n"
              "                 if(!__ptr && 100 != 0)\n"
              "                 {\n"
              "                     exit(1);\n"
              "                 }\n"
              "                 __ptr;\n"
              "                } );\n"
              "    free(sym);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void foo()\n"
              "{\n"
              "    void *sym = ( {\n"
              "                 void *__ptr = g_malloc(100);\n"
              "                 if(!__ptr && 100 != 0)\n"
              "                 {\n"
              "                     g_exit(1);\n"
              "                 }\n"
              "                 __ptr;\n"
              "                } );\n"
              "    g_free(sym);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void knownFunctions() {
        check("void foo()\n"
              "{\n"
              "    int *p = new int[100];\n"
              "    typeid(p);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: p\n", errout.str());
    }

    void same_function_name() {
        check("void a(char *p)\n"
              "{ }\n"
              "void b()\n"
              "{\n"
              "    char *p = malloc(10);\n"
              "    abc.a(p);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void functionParameter() {
        check("void a(char *p)\n"
              "{\n"
              "    p = malloc(100);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Memory leak: p\n", errout.str());
    }

    // Ticket #2014 - setjmp / longjmp
    void jmp() {
        check("int main()\n"
              "{\n"
              "  jmp_buf env;\n"
              "  int val;\n"
              "  char *a;\n"
              "\n"
              "  val = setjmp(env);\n"
              "  if(val)\n"
              "  {\n"
              "    delete a;\n"
              "    return 0;\n"
              "  }\n"
              "\n"
              "  a = new char(1);\n"
              "  longjmp(env, 1);\n"
              "\n"
              "  return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void trac1949() {
        check("int fn()\n"
              "{\n"
              "  char * buff = new char[100];\n"
              "  assert (buff);\n"
              "  return 0;\n"
              "}"
             );
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: buff\n", errout.str());
    }

    void trac2540() {
        check("void f()\n"
              "{\n"
              "    char* str = strdup(\"abc def\");\n"
              "    char *name = strtok(str, \" \");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: str\n", errout.str());

        check("void f(char *cBuf)\n"
              "{\n"
              "    char* str = strdup(*cBuf);\n"
              "    char *name = strtok(str, \" \");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: str\n", errout.str());
    }

    void trac2662() {
        // segfault because of endless recursion
        // call_func -> getAllocationType -> functionReturnType -> call_func ..

        check("char *foo() {\n"
              "    return foo();\n"
              "}\n"
              "\n"
              "void bar() {\n"
              "    char *s = foo();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("char *foo() {\n"
              "    char *s = foo();\n"
              "    return s;\n"
              "}\n"
              "\n"
              "void bar() {\n"
              "    char *s = foo();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int shl()\n"
              "{\n"
              "    int a = shr();\n"
              "    return a;\n"
              "}\n"
              "\n"
              "int shr()\n"
              "{\n"
              "    return shl();\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void trac1879() {
        // #1879 non regression test case
        check("void test() {\n"
              "int *a = new int[10];\n"
              "try {}\n"
              "catch(...) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: a\n", errout.str());
    }

    void ptrptr() {
        check("void f() {\n"
              "    char *p;\n"
              "    char **pp = &p;\n"
              "    *pp = calloc(10);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: p\n", errout.str());
    }

    void c_code() {
        check("int main(void) {\n"
              "    struct llist *ll = malloc(sizeof(struct llist));\n"
              "    free(ll);\n"
              "    ll = NULL;\n"
              "    delete(ll, ll->top);\n"
              "}", nullptr, true);
        ASSERT_EQUALS("", errout.str());
    }

    void gnucfg() {
        Settings settings;
        settings.standards.posix = true;
        LOAD_LIB_2(settings.library, "gnu.cfg");
        const char code[] = "void leak() {\n"
                            "  char * p = get_current_dir_name();\n" // memory leak
                            "}\n"
                            "void noLeak() {\n"
                            "  char * p = get_current_dir_name();\n"
                            "  free(p)\n;"
                            "}";
        check(code, &settings);
        ASSERT_EQUALS("[test.cpp:3]: (error) Memory leak: p\n", errout.str());
    }
};

static TestMemleakInFunction testMemleakInFunction;
















class TestMemleakInClass : public TestFixture {
public:
    TestMemleakInClass() : TestFixture("TestMemleakInClass") {
    }

private:
    /**
     * Tokenize and execute leak check for given code
     * @param code Source code
     */
    void check(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("warning");
        settings.addEnabled("style");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        // Check for memory leaks..
        CheckMemoryLeakInClass checkMemoryLeak(&tokenizer, &settings, this);
        checkMemoryLeak.check();
    }

    void run() {
        TEST_CASE(class1);
        TEST_CASE(class2);
        TEST_CASE(class3);
        TEST_CASE(class4);
        TEST_CASE(class6);
        TEST_CASE(class7);
        TEST_CASE(class8);
        TEST_CASE(class9);
        TEST_CASE(class10);
        TEST_CASE(class11);
        TEST_CASE(class12);
        TEST_CASE(class13);
        TEST_CASE(class14);
        TEST_CASE(class15);
        TEST_CASE(class16);
        TEST_CASE(class17);
        TEST_CASE(class18);
        TEST_CASE(class19); // ticket #2219
        TEST_CASE(class20);
        TEST_CASE(class21); // ticket #2517
        TEST_CASE(class22); // ticket #3012
        TEST_CASE(class23); // ticket #3303
        TEST_CASE(class24); // ticket #3806 - false positive in copy constructor
        TEST_CASE(class25); // ticket #4367 - false positive implementation for destructor is not seen

        TEST_CASE(staticvar);

        TEST_CASE(free_member_in_sub_func);

        TEST_CASE(mismatch1);
        TEST_CASE(mismatch2); // #5659

        // allocating member variable in public function
        TEST_CASE(func1);
        TEST_CASE(func2);
    }


    void class1() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char *str1;\n"
              "    char *str2;\n"
              "public:\n"
              "    Fred();\n"
              "    ~Fred();\n"
              "};\n"
              "\n"
              "Fred::Fred()\n"
              "{\n"
              "    str1 = new char[10];\n"
              "    str2 = new char[10];\n"
              "}\n"
              "\n"
              "Fred::~Fred()\n"
              "{\n"
              "    delete [] str2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Class 'Fred' is unsafe, 'Fred::str1' can leak by wrong usage.\n", errout.str());

        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char *str1;\n"
              "    char *str2;\n"
              "public:\n"
              "    Fred()\n"
              "    {\n"
              "        str1 = new char[10];\n"
              "        str2 = new char[10];\n"
              "    }\n"
              "    ~Fred()\n"
              "    {\n"
              "        delete [] str2;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Class 'Fred' is unsafe, 'Fred::str1' can leak by wrong usage.\n", errout.str());
    }

    void class2() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char *str1;\n"
              "public:\n"
              "    Fred();\n"
              "    ~Fred();\n"
              "};\n"
              "\n"
              "Fred::Fred()\n"
              "{\n"
              "    str1 = new char[10];\n"
              "}\n"
              "\n"
              "Fred::~Fred()\n"
              "{\n"
              "    free(str1);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:17]: (error) Mismatching allocation and deallocation: Fred::str1\n", errout.str());
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    gchar *str1;\n"
              "public:\n"
              "    Fred();\n"
              "    ~Fred();\n"
              "};\n"
              "\n"
              "Fred::Fred()\n"
              "{\n"
              "    str1 = new char[10];\n"
              "}\n"
              "\n"
              "Fred::~Fred()\n"
              "{\n"
              "    g_free(str1);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:17]: (error) Mismatching allocation and deallocation: Fred::str1\n", "", errout.str());

        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char *str1;\n"
              "public:\n"
              "    Fred()\n"
              "    {\n"
              "        str1 = new char[10];\n"
              "    }\n"
              "    ~Fred()\n"
              "    {\n"
              "        free(str1);\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:12]: (error) Mismatching allocation and deallocation: Fred::str1\n", errout.str());
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char *str1;\n"
              "public:\n"
              "    Fred()\n"
              "    {\n"
              "        str1 = new char[10];\n"
              "    }\n"
              "    ~Fred()\n"
              "    {\n"
              "        g_free(str1);\n"
              "    }\n"
              "};");
        TODO_ASSERT_EQUALS("[test.cpp:12]: (error) Mismatching allocation and deallocation: Fred::str1\n", "", errout.str());
    }

    void class3() {
        check("class Token;\n"
              "\n"
              "class Tokenizer\n"
              "{\n"
              "private:\n"
              "    Token *_tokens;\n"
              "\n"
              "public:\n"
              "    Tokenizer();\n"
              "    ~Tokenizer();\n"
              "    void deleteTokens(Token *tok);\n"
              "};\n"
              "\n"
              "Tokenizer::Tokenizer()\n"
              "{\n"
              "    _tokens = new Token;\n"
              "}\n"
              "\n"
              "Tokenizer::~Tokenizer()\n"
              "{\n"
              "    deleteTokens(_tokens);\n"
              "}\n"
              "\n"
              "void Tokenizer::deleteTokens(Token *tok)\n"
              "{\n"
              "    while (tok)\n"
              "    {\n"
              "        Token *next = tok->next();\n"
              "        delete tok;\n"
              "        tok = next;\n"
              "    }\n"
              "}");

        ASSERT_EQUALS("", errout.str());

        check("class Token;\n"
              "\n"
              "class Tokenizer\n"
              "{\n"
              "private:\n"
              "    Token *_tokens;\n"
              "\n"
              "public:\n"
              "    Tokenizer()\n"
              "    {\n"
              "        _tokens = new Token;\n"
              "    }\n"
              "    ~Tokenizer()\n"
              "    {\n"
              "        deleteTokens(_tokens);\n"
              "    }\n"
              "    void deleteTokens(Token *tok)\n"
              "    {\n"
              "        while (tok)\n"
              "        {\n"
              "            Token *next = tok->next();\n"
              "            delete tok;\n"
              "            tok = next;\n"
              "        }\n"
              "    }\n"
              "};");

        ASSERT_EQUALS("", errout.str());
    }

    void class4() {
        check("struct ABC;\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "    void addAbc(ABC *abc);\n"
              "public:\n"
              "    void click();\n"
              "};\n"
              "\n"
              "void Fred::addAbc(ABC* abc)\n"
              "{\n"
              "    AbcPosts->Add(abc);\n"
              "}\n"
              "\n"
              "void Fred::click()\n"
              "{\n"
              "    ABC *p = new ABC;\n"
              "    addAbc( p );\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("struct ABC;\n"
              "class Fred\n"
              "{\n"
              "private:\n"
              "    void addAbc(ABC* abc)\n"
              "    {\n"
              "        AbcPosts->Add(abc);\n"
              "    }\n"
              "public:\n"
              "    void click()\n"
              "    {\n"
              "        ABC *p = new ABC;\n"
              "        addAbc( p );\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void class6() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    void foo();\n"
              "};\n"
              "\n"
              "void Fred::foo()\n"
              "{\n"
              "    char *str = new char[100];\n"
              "    delete [] str;\n"
              "    hello();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    void foo()\n"
              "    {\n"
              "        char *str = new char[100];\n"
              "        delete [] str;\n"
              "        hello();\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void class7() {
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    int *i;\n"
              "    Fred();\n"
              "    ~Fred();\n"
              "};\n"
              "\n"
              "Fred::Fred()\n"
              "{\n"
              "    this->i = new int;\n"
              "}\n"
              "Fred::~Fred()\n"
              "{\n"
              "    delete this->i;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    int *i;\n"
              "    Fred()\n"
              "    {\n"
              "        this->i = new int;\n"
              "    }\n"
              "    ~Fred()\n"
              "    {\n"
              "        delete this->i;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void class8() {
        check("class A\n"
              "{\n"
              "public:\n"
              "    void a();\n"
              "    void doNothing() { }\n"
              "};\n"
              "\n"
              "void A::a()\n"
              "{\n"
              "    int* c = new int(1);\n"
              "    delete c;\n"
              "    doNothing(c);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class A\n"
              "{\n"
              "public:\n"
              "    void a()\n"
              "    {\n"
              "        int* c = new int(1);\n"
              "        delete c;\n"
              "        doNothing(c);\n"
              "    }\n"
              "    void doNothing() { }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void class9() {
        check("class A\n"
              "{\n"
              "public:\n"
              "    int * p;\n"
              "    A();\n"
              "    ~A();\n"
              "};\n"
              "\n"
              "A::A()\n"
              "{ p = new int; }\n"
              "\n"
              "A::~A()\n"
              "{ delete (p); }");
        ASSERT_EQUALS("", errout.str());

        check("class A\n"
              "{\n"
              "public:\n"
              "    int * p;\n"
              "    A()\n"
              "    { p = new int; }\n"
              "    ~A()\n"
              "    { delete (p); }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void class10() {
        check("class A\n"
              "{\n"
              "public:\n"
              "    int * p;\n"
              "    A();\n"
              "};\n"
              "A::A()\n"
              "{ p = new int; }");
        ASSERT_EQUALS("[test.cpp:4]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());

        check("class A\n"
              "{\n"
              "public:\n"
              "    int * p;\n"
              "    A() { p = new int; }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());
    }

    void class11() {
        check("class A\n"
              "{\n"
              "public:\n"
              "    int * p;\n"
              "    A() : p(new int[10])\n"
              "    { }"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());

        check("class A\n"
              "{\n"
              "public:\n"
              "    int * p;\n"
              "    A();\n"
              "};\n"
              "A::A() : p(new int[10])\n"
              "{ }");
        ASSERT_EQUALS("[test.cpp:4]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());
    }

    void class12() {
        check("class A\n"
              "{\n"
              "private:\n"
              "    int *p;\n"
              "public:\n"
              "    A();\n"
              "    ~A();\n"
              "    void cleanup();"
              "};\n"
              "\n"
              "A::A()\n"
              "{ p = new int[10]; }\n"
              "\n"
              "A::~A()\n"
              "{ }\n"
              "\n"
              "void A::cleanup()\n"
              "{ delete [] p; }");
        ASSERT_EQUALS("[test.cpp:4]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());

        check("class A\n"
              "{\n"
              "private:\n"
              "    int *p;\n"
              "public:\n"
              "    A()\n"
              "    { p = new int[10]; }\n"
              "    ~A()\n"
              "    { }\n"
              "    void cleanup()\n"
              "    { delete [] p; }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:4]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());
    }

    void class13() {
        check("class A\n"
              "{\n"
              "private:\n"
              "    int *p;\n"
              "public:\n"
              "    A();\n"
              "    ~A();\n"
              "    void foo();"
              "};\n"
              "\n"
              "A::A()\n"
              "{ }\n"
              "\n"
              "A::~A()\n"
              "{ }\n"
              "\n"
              "void A::foo()\n"
              "{ p = new int[10]; delete [] p; }");
        ASSERT_EQUALS("[test.cpp:17]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated.\n", errout.str());

        check("class A\n"
              "{\n"
              "private:\n"
              "    int *p;\n"
              "public:\n"
              "    A()\n"
              "    { }\n"
              "    ~A()\n"
              "    { }\n"
              "    void foo()\n"
              "    { p = new int[10]; delete [] p; }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:11]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated.\n", errout.str());
    }

    void class14() {
        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    void init();\n"
              "};\n"
              "\n"
              "void A::init()\n"
              "{ p = new int[10]; }");
        ASSERT_EQUALS("[test.cpp:9]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated.\n"
                      "[test.cpp:3]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());

        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    void init()\n"
              "    { p = new int[10]; }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated.\n"
                      "[test.cpp:3]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());


        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    void init();\n"
              "};\n"
              "\n"
              "void A::init()\n"
              "{ p = new int; }");
        ASSERT_EQUALS("[test.cpp:9]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated.\n"
                      "[test.cpp:3]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());

        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    void init()\n"
              "    { p = new int; }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated.\n"
                      "[test.cpp:3]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());


        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    void init();\n"
              "};\n"
              "\n"
              "void A::init()\n"
              "{ p = malloc(sizeof(int)*10); }");
        ASSERT_EQUALS("[test.cpp:9]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated.\n"
                      "[test.cpp:3]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());

        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    void init()\n"
              "    { p = malloc(sizeof(int)*10); }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Possible leak in public function. The pointer 'p' is not deallocated before it is allocated.\n"
                      "[test.cpp:3]: (style) Class 'A' is unsafe, 'A::p' can leak by wrong usage.\n", errout.str());
    }

    void class15() {
        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    A();\n"
              "    ~A() { delete [] p; }\n"
              "};\n"
              "A::A()\n"
              "{ p = new int[10]; }");
        ASSERT_EQUALS("", errout.str());

        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    A()\n"
              "    { p = new int[10]; }\n"
              "    ~A() { delete [] p; }\n"
              "};");
        ASSERT_EQUALS("", errout.str());


        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    A();\n"
              "    ~A() { delete p; }\n"
              "};\n"
              "A::A()\n"
              "{ p = new int; }");
        ASSERT_EQUALS("", errout.str());

        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    A()\n"
              "    { p = new int; }\n"
              "    ~A() { delete p; }\n"
              "};");
        ASSERT_EQUALS("", errout.str());


        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    A();\n"
              "    ~A() { free(p); }\n"
              "};\n"
              "A::A()\n"
              "{ p = malloc(sizeof(int)*10); }");
        ASSERT_EQUALS("", errout.str());
        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    A();\n"
              "    ~A() { g_free(p); }\n"
              "};\n"
              "A::A()\n"
              "{ p = g_malloc(sizeof(int)*10); }");
        ASSERT_EQUALS("", errout.str());

        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    A()\n"
              "    { p = malloc(sizeof(int)*10); }\n"
              "    ~A() { free(p); }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
        check("class A\n"
              "{\n"
              "    int *p;\n"
              "public:\n"
              "    A()\n"
              "    { p = g_malloc(sizeof(int)*10); }\n"
              "    ~A() { g_free(p); }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void class16() {
        // Ticket #1510
        check("class A\n"
              "{\n"
              "    int *a;\n"
              "    int *b;\n"
              "public:\n"
              "    A() { a = b = new int[10]; }\n"
              "    ~A() { delete [] a; }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void class17() {
        // Ticket #1557
        check("class A {\n"
              "private:\n"
              "    char *pd;\n"
              "public:\n"
              "    void foo();\n"
              "};\n"
              "\n"
              "void A::foo()\n"
              "{\n"
              "    A::pd = new char[12];\n"
              "    delete [] A::pd;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (warning) Possible leak in public function. The pointer 'pd' is not deallocated before it is allocated.\n", errout.str());

        check("class A {\n"
              "private:\n"
              "    char *pd;\n"
              "public:\n"
              "    void foo()\n"
              "    {\n"
              "        pd = new char[12];\n"
              "        delete [] pd;\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:6]: (warning) Possible leak in public function. The pointer 'pd' is not deallocated before it is allocated.\n", errout.str());

        check("class A {\n"
              "private:\n"
              "    char *pd;\n"
              "public:\n"
              "    void foo();\n"
              "};\n"
              "\n"
              "void A::foo()\n"
              "{\n"
              "    pd = new char[12];\n"
              "    delete [] pd;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (warning) Possible leak in public function. The pointer 'pd' is not deallocated before it is allocated.\n", errout.str());
    }

    void class18() {
        // Ticket #853
        check("class  A : public x\n"
              "{\n"
              "public:\n"
              "  A()\n"
              "  {\n"
              "    a = new char[10];\n"
              "    foo(a);\n"
              "  }\n"
              "private:\n"
              "  char *a;\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        check("class  A : public x\n"
              "{\n"
              "public:\n"
              "  A();\n"
              "private:\n"
              "  char *a;\n"
              "};\n"
              "A::A()\n"
              "{\n"
              "  a = new char[10];\n"
              "  foo(a);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void class19() {
        // Ticket #2219
        check("class Foo\n"
              "{\n"
              "private:\n"
              "    TRadioButton* rp1;\n"
              "    TRadioButton* rp2;\n"
              "public:\n"
              "    Foo();\n"
              "};\n"
              "Foo::Foo()\n"
              "{\n"
              "    rp1 = new TRadioButton(this);\n"
              "    rp2 = new TRadioButton(this);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class TRadioButton { };\n"
              "class Foo\n"
              "{\n"
              "private:\n"
              "    TRadioButton* rp1;\n"
              "    TRadioButton* rp2;\n"
              "public:\n"
              "    Foo();\n"
              "};\n"
              "Foo::Foo()\n"
              "{\n"
              "    rp1 = new TRadioButton;\n"
              "    rp2 = new TRadioButton;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Class 'Foo' is unsafe, 'Foo::rp1' can leak by wrong usage.\n"
                      "[test.cpp:6]: (style) Class 'Foo' is unsafe, 'Foo::rp2' can leak by wrong usage.\n", errout.str());

        check("class TRadioButton { };\n"
              "class Foo\n"
              "{\n"
              "private:\n"
              "    TRadioButton* rp1;\n"
              "    TRadioButton* rp2;\n"
              "public:\n"
              "    Foo();\n"
              "    ~Foo();\n"
              "};\n"
              "Foo::Foo()\n"
              "{\n"
              "    rp1 = new TRadioButton;\n"
              "    rp2 = new TRadioButton;\n"
              "}\n"
              "Foo::~Foo()\n"
              "{\n"
              "    delete rp1;\n"
              "    delete rp2;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void class20() {
        check("namespace ns1 {\n"
              "    class Fred\n"
              "    {\n"
              "    private:\n"
              "        char *str1;\n"
              "        char *str2;\n"
              "    public:\n"
              "        Fred()\n"
              "        {\n"
              "            str1 = new char[10];\n"
              "            str2 = new char[10];\n"
              "        }\n"
              "        ~Fred()\n"
              "        {\n"
              "            delete [] str2;\n"
              "        }\n"
              "    };\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Class 'Fred' is unsafe, 'Fred::str1' can leak by wrong usage.\n", errout.str());

        check("namespace ns1 {\n"
              "    class Fred\n"
              "    {\n"
              "    private:\n"
              "        char *str1;\n"
              "        char *str2;\n"
              "    public:\n"
              "        Fred();\n"
              "        ~Fred();\n"
              "    };\n"
              "\n"
              "    Fred::Fred()\n"
              "    {\n"
              "        str1 = new char[10];\n"
              "        str2 = new char[10];\n"
              "    }\n"
              "\n"
              "    Fred::~Fred()\n"
              "    {\n"
              "        delete [] str2;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Class 'Fred' is unsafe, 'Fred::str1' can leak by wrong usage.\n", errout.str());

        check("namespace ns1 {\n"
              "    class Fred\n"
              "    {\n"
              "    private:\n"
              "        char *str1;\n"
              "        char *str2;\n"
              "    public:\n"
              "        Fred();\n"
              "        ~Fred();\n"
              "    };\n"
              "}\n"
              "ns1::Fred::Fred()\n"
              "{\n"
              "    str1 = new char[10];\n"
              "    str2 = new char[10];\n"
              "}\n"
              "\n"
              "ns1::Fred::~Fred()\n"
              "{\n"
              "    delete [] str2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Class 'Fred' is unsafe, 'Fred::str1' can leak by wrong usage.\n", errout.str());

        check("namespace ns1 {\n"
              "    namespace ns2 {\n"
              "        class Fred\n"
              "        {\n"
              "        private:\n"
              "            char *str1;\n"
              "            char *str2;\n"
              "        public:\n"
              "            Fred();\n"
              "            ~Fred();\n"
              "        };\n"
              "    }\n"
              "}\n"
              "ns1::ns2::Fred::Fred()\n"
              "{\n"
              "    str1 = new char[10];\n"
              "    str2 = new char[10];\n"
              "}\n"
              "\n"
              "ns1::ns2::Fred::~Fred()\n"
              "{\n"
              "    delete [] str2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (style) Class 'Fred' is unsafe, 'Fred::str1' can leak by wrong usage.\n", errout.str());

        check("namespace ns1 {\n"
              "    namespace ns2 {\n"
              "        namespace ns3 {\n"
              "            class Fred\n"
              "            {\n"
              "            private:\n"
              "                char *str1;\n"
              "                char *str2;\n"
              "            public:\n"
              "                Fred();\n"
              "                ~Fred();\n"
              "            };\n"
              "        }\n"
              "    }\n"
              "}\n"
              "ns1::ns2::ns3::Fred::Fred()\n"
              "{\n"
              "    str1 = new char[10];\n"
              "    str2 = new char[10];\n"
              "}\n"
              "\n"
              "ns1::ns2::ns3::Fred::~Fred()\n"
              "{\n"
              "    delete [] str2;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (style) Class 'Fred' is unsafe, 'Fred::str1' can leak by wrong usage.\n", errout.str());
    }

    void class21() { // ticket #2517
        check("struct B { };\n"
              "struct C\n"
              "{\n"
              "    B * b;\n"
              "    C(B * x) : b(x) { }\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B *b;\n"
              "    C *c;\n"
              "public:\n"
              "    A() : b(new B()), c(new C(b)) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (style) Class 'A' is unsafe, 'A::b' can leak by wrong usage.\n"
                      "[test.cpp:10]: (style) Class 'A' is unsafe, 'A::c' can leak by wrong usage.\n", errout.str());

        check("struct B { };\n"
              "struct C\n"
              "{\n"
              "    B * b;\n"
              "    C(B * x) : b(x) { }\n"
              "};\n"
              "class A\n"
              "{\n"
              "    B *b;\n"
              "    C *c;\n"
              "public:\n"
              "    A()\n"
              "    {\n"
              "       b = new B();\n"
              "       c = new C(b);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (style) Class 'A' is unsafe, 'A::b' can leak by wrong usage.\n"
                      "[test.cpp:10]: (style) Class 'A' is unsafe, 'A::c' can leak by wrong usage.\n", errout.str());
    }

    void class22() { // ticket #3012 - false positive
        check("class Fred {\n"
              "private:\n"
              "    int * a;\n"
              "private:\n"
              "    Fred() { a = new int; }\n"
              "    ~Fred() { (delete(a), (a)=NULL); }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void class23() { // ticket #3303 - false positive
        check("class CDataImpl {\n"
              "public:\n"
              "    CDataImpl() { m_refcount = 1; }\n"
              "    void Release() { if (--m_refcount == 0) delete this; }\n"
              "private:\n"
              "    int m_refcount;\n"
              "};\n"
              "\n"
              "class CData {\n"
              "public:\n"
              "    CData() : m_impl(new CDataImpl()) { }\n"
              "    ~CData() { if (m_impl) m_impl->Release(); }\n"
              "private:\n"
              "    CDataImpl *m_impl;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void class24() { // ticket #3806 - false positive in copy constructor
        check("class Fred {\n"
              "private:\n"
              "    int * a;\n"
              "public:\n"
              "    Fred(const Fred &fred) { a = new int; }\n"
              "    ~Fred() { delete a; }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void class25() { // ticket #4367 - false positive when implementation for destructor is not seen
        check("class Fred {\n"
              "private:\n"
              "    int * a;\n"
              "public:\n"
              "    Fred() { a = new int; }\n"
              "    ~Fred();\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void staticvar() {
        check("class A\n"
              "{\n"
              "private:\n"
              "    static int * p;\n"
              "public:"
              "    A()\n"
              "    {\n"
              "        if (!p)\n"
              "            p = new int[100];\n"
              "    }\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }


    void free_member_in_sub_func() {
        // Member function
        check("class Tokenizer\n"
              "{\n"
              "public:\n"
              "    Tokenizer();\n"
              "    ~Tokenizer();\n"
              "\n"
              "private:\n"
              "    int *_tokens;\n"
              "    static void deleteTokens(int *tok);\n"
              "};\n"
              "\n"
              "Tokenizer::Tokenizer()\n"
              "{\n"
              "     _tokens = new int;\n"
              "}\n"
              "\n"
              "Tokenizer::~Tokenizer()\n"
              "{\n"
              "    deleteTokens(_tokens);\n"
              "    _tokens = 0;\n"
              "}\n"
              "\n"
              "void Tokenizer::deleteTokens(int *tok)\n"
              "{\n"
              "    delete tok;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Global function
        check("void deleteTokens(int *tok)\n"
              "{\n"
              "    delete tok;\n"
              "}\n"
              "class Tokenizer\n"
              "{\n"
              "public:\n"
              "    Tokenizer();\n"
              "    ~Tokenizer();\n"
              "\n"
              "private:\n"
              "    int *_tokens;\n"
              "};\n"
              "\n"
              "Tokenizer::Tokenizer()\n"
              "{\n"
              "     _tokens = new int;\n"
              "}\n"
              "\n"
              "Tokenizer::~Tokenizer()\n"
              "{\n"
              "    deleteTokens(_tokens);\n"
              "    _tokens = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void mismatch1() {
        check("class A\n"
              "{\n"
              "public:\n"
              "    A(int i);\n"
              "    ~A();\n"
              "private:\n"
              "    char* pkt_buffer;\n"
              "};\n"
              "\n"
              "A::A(int i)\n"
              "{\n"
              "    pkt_buffer = new char[8192];\n"
              "    if (i != 1) {\n"
              "        delete pkt_buffer;\n"
              "        pkt_buffer = 0;\n"
              "    }\n"
              "}\n"
              "\n"
              "A::~A() {\n"
              "    delete [] pkt_buffer;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:14]: (error) Mismatching allocation and deallocation: A::pkt_buffer\n", errout.str());
    }

    void mismatch2() { // #5659
        check("namespace NS\n"
              "{\n"
              "class Foo\n"
              "{\n"
              "public:\n"
              "  void fct();\n"
              "\n"
              "private:\n"
              "  char* data_;\n"
              "};\n"
              "}\n"
              "\n"
              "using namespace NS;\n"
              "\n"
              "void Foo::fct()\n"
              "{\n"
              "  data_ = new char[42];\n"
              "  delete data_;\n"
              "  data_ = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:16]: (warning) Possible leak in public function. The pointer 'data_' is not deallocated before it is allocated.\n"
                      "[test.cpp:18]: (error) Mismatching allocation and deallocation: Foo::data_\n", errout.str());

        check("namespace NS\n"
              "{\n"
              "class Foo\n"
              "{\n"
              "public:\n"
              "  void fct(int i);\n"
              "\n"
              "private:\n"
              "  char* data_;\n"
              "};\n"
              "}\n"
              "\n"
              "using namespace NS;\n"
              "\n"
              "void Foo::fct(int i)\n"
              "{\n"
              "  data_ = new char[42];\n"
              "  delete data_;\n"
              "  data_ = 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:16]: (warning) Possible leak in public function. The pointer 'data_' is not deallocated before it is allocated.\n"
                      "[test.cpp:18]: (error) Mismatching allocation and deallocation: Foo::data_\n", errout.str());
    }

    void func1() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char *s;\n"
              "public:\n"
              "    Fred() { s = 0; }\n"
              "    ~Fred() { free(s); }\n"
              "    void xy()\n"
              "    { s = malloc(100); }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:9]: (warning) Possible leak in public function. The pointer 's' is not deallocated before it is allocated.\n", errout.str());
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    gchar *s;\n"
              "public:\n"
              "    Fred() { s = 0; }\n"
              "    ~Fred() { g_free(s); }\n"
              "    void xy()\n"
              "    { s = g_malloc(100); }\n"
              "};");
        TODO_ASSERT_EQUALS("[test.cpp:9]: (warning) Possible leak in public function. The pointer 's' is not deallocated before it is allocated.\n", "", errout.str());

        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { s = 0; }\n"
              "    ~Fred() { free(s); }\n"
              "    void xy()\n"
              "    { s = malloc(100); }\n"
              "private:\n"
              "    char *s;\n"
              "};");
        ASSERT_EQUALS("[test.cpp:7]: (warning) Possible leak in public function. The pointer 's' is not deallocated before it is allocated.\n", errout.str());
        check("class Fred\n"
              "{\n"
              "public:\n"
              "    Fred() { s = 0; }\n"
              "    ~Fred() { g_free(s); }\n"
              "    void xy()\n"
              "    { s = g_malloc(100); }\n"
              "private:\n"
              "    char *s;\n"
              "};");
        TODO_ASSERT_EQUALS("[test.cpp:7]: (warning) Possible leak in public function. The pointer 's' is not deallocated before it is allocated.\n", "", errout.str());
    }

    void func2() {
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    char *s;\n"
              "public:\n"
              "    Fred() { s = 0; }\n"
              "    ~Fred() { free(s); }\n"
              "    const Fred & operator = (const Fred &f)\n"
              "    { s = malloc(100); }\n"
              "};");
        ASSERT_EQUALS("[test.cpp:9]: (warning) Possible leak in public function. The pointer 's' is not deallocated before it is allocated.\n", errout.str());
        check("class Fred\n"
              "{\n"
              "private:\n"
              "    gchar *s;\n"
              "public:\n"
              "    Fred() { s = 0; }\n"
              "    ~Fred() { g_free(s); }\n"
              "    const Fred & operator = (const Fred &f)\n"
              "    { s = g_malloc(100); }\n"
              "};");
        TODO_ASSERT_EQUALS("[test.cpp:9]: (warning) Possible leak in public function. The pointer 's' is not deallocated before it is allocated.\n", "", errout.str());
    }
};

static TestMemleakInClass testMemleakInClass;







class TestMemleakStructMember : public TestFixture {
public:
    TestMemleakStructMember() : TestFixture("TestMemleakStructMember") {
    }

private:
    void check(const char code[], const char fname[] = 0, bool isCPP = true) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, fname ? fname : (isCPP ? "test.cpp" : "test.c"));
        tokenizer.simplifyTokenList2();

        // Check for memory leaks..
        CheckMemoryLeakStructMember checkMemoryLeakStructMember(&tokenizer, &settings, this);
        checkMemoryLeakStructMember.check();
    }

    void run() {
        // testing that errors are detected
        TEST_CASE(err);

        // handle / bail out when "goto" is found
        TEST_CASE(goto_);

        // Don't report errors if the struct is returned
        TEST_CASE(ret1);
        TEST_CASE(ret2);

        // assignments
        TEST_CASE(assign1);
        TEST_CASE(assign2);
        TEST_CASE(assign3);

        // Failed allocation
        TEST_CASE(failedAllocation);

        TEST_CASE(function1);   // Deallocating in function
        TEST_CASE(function2);   // #2848: Taking address in function
        TEST_CASE(function3);   // #3024: kernel list
        TEST_CASE(function4);   // #3038: Deallocating in function

        // Handle if-else
        TEST_CASE(ifelse);

        // Linked list
        TEST_CASE(linkedlist);

        // struct variable is a global variable
        TEST_CASE(globalvar);

        // local struct variable
        TEST_CASE(localvars);

        // Segmentation fault in CheckMemoryLeakStructMember
        TEST_CASE(trac5030);

        TEST_CASE(varid); // #5201: Analysis confused by (variable).attribute notation
        TEST_CASE(varid_2); // #5315: Analysis confused by ((variable).attribute) notation
    }

    void err() {
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "    free(abc);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: abc.a\n", errout.str());
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = g_malloc(sizeof(struct ABC));\n"
              "    abc->a = g_malloc(10);\n"
              "    g_free(abc);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: abc.a\n", "", errout.str());

        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: abc.a\n", errout.str());
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = g_malloc(sizeof(struct ABC));\n"
              "    abc->a = g_malloc(10);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:5]: (error) Memory leak: abc.a\n", "", errout.str());

        check("static ABC * foo()\n"
              "{\n"
              "    ABC *abc = malloc(sizeof(ABC));\n"
              "    abc->a = malloc(10);\n"
              "    abc->b = malloc(10);\n"
              "    if (abc->b == 0)\n"
              "    {\n"
              "        return 0;\n"
              "    }\n"
              "    return abc;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (error) Memory leak: abc.a\n", errout.str());
        check("static ABC * foo()\n"
              "{\n"
              "    ABC *abc = g_malloc(sizeof(ABC));\n"
              "    abc->a = g_malloc(10);\n"
              "    abc->b = g_malloc(10);\n"
              "    if (abc->b == 0)\n"
              "    {\n"
              "        return 0;\n"
              "    }\n"
              "    return abc;\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:8]: (error) Memory leak: abc.a\n", "", errout.str());

        check("static void foo(int a)\n"
              "{\n"
              "    ABC *abc = malloc(sizeof(ABC));\n"
              "    abc->a = malloc(10);\n"
              "    if (a == 1)\n"
              "    {\n"
              "        free(abc->a);\n"
              "        return;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (error) Memory leak: abc.a\n", errout.str());
        check("static void foo(int a)\n"
              "{\n"
              "    ABC *abc = g_malloc(sizeof(ABC));\n"
              "    abc->a = g_malloc(10);\n"
              "    if (a == 1)\n"
              "    {\n"
              "        g_free(abc->a);\n"
              "        return;\n"
              "    }\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:10]: (error) Memory leak: abc.a\n", "", errout.str());
    }

    void goto_() {
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "    if (abc->a)\n"
              "    { goto out; }\n"
              "    free(abc);\n"
              "    return;\n"
              "out:\n"
              "    free(abc->a);\n"
              "    free(abc);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = g_malloc(sizeof(struct ABC));\n"
              "    abc->a = g_malloc(10);\n"
              "    if (abc->a)\n"
              "    { goto out; }\n"
              "    g_free(abc);\n"
              "    return;\n"
              "out:\n"
              "    g_free(abc->a);\n"
              "    g_free(abc);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ret1() {
        check("static ABC * foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "    return abc;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("static ABC * foo()\n"
              "{\n"
              "    struct ABC *abc = g_malloc(sizeof(struct ABC));\n"
              "    abc->a = g_malloc(10);\n"
              "    return abc;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("static void foo(struct ABC *abc)\n"
              "{\n"
              "    abc->a = malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("static void foo(struct ABC *abc)\n"
              "{\n"
              "    abc->a = g_malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ret2() {
        check("static ABC * foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "    return &abc->self;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("static ABC * foo()\n"
              "{\n"
              "    struct ABC *abc = g_malloc(sizeof(struct ABC));\n"
              "    abc->a = g_malloc(10);\n"
              "    return &abc->self;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void assign1() {
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = abc1;\n"
              "    abc->a = malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = abc1;\n"
              "    abc->a = g_malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc;\n"
              "    abc1 = abc = malloc(sizeof(ABC));\n"
              "    abc->a = malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc;\n"
              "    abc1 = abc = g_malloc(sizeof(ABC));\n"
              "    abc->a = g_malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout.str());


        check("static void foo()\n"
              "{\n"
              " struct msn_entry *ptr;\n"
              " ptr = malloc(sizeof(struct msn_entry));\n"
              " ptr->msn = malloc(100);\n"
              " back = ptr;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("static void foo()\n"
              "{\n"
              " struct msn_entry *ptr;\n"
              " ptr = g_malloc(sizeof(struct msn_entry));\n"
              " ptr->msn = g_malloc(100);\n"
              " back = ptr;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

    }

    void assign2() {
        check("static void foo() {\n"
              "    struct ABC *abc = malloc(123);\n"
              "    abc->a = abc->b = malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("static void foo() {\n"
              "    struct ABC *abc = g_malloc(123);\n"
              "    abc->a = abc->b = g_malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void assign3() {
        check("void f(struct s *f1) {\n"
              "    struct s f2;\n"
              "    f2.a = malloc(100);\n"
              "    *f1 = f2;\n"
              "}\n", "test.c");
        ASSERT_EQUALS("", errout.str());
        check("void f(struct s *f1) {\n"
              "    struct s f2;\n"
              "    f2.a = g_malloc(100);\n"
              "    *f1 = f2;\n"
              "}\n", "test.c");
        ASSERT_EQUALS("", errout.str());
    }

    void failedAllocation() {
        check("static struct ABC * foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "    if (!abc->a)\n"
              "    {\n"
              "        free(abc);\n"
              "        return 0;\n"
              "    }\n"
              "    return abc;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("static struct ABC * foo()\n"
              "{\n"
              "    struct ABC *abc = g_malloc(sizeof(struct ABC));\n"
              "    abc->a = g_malloc(10);\n"
              "    if (!abc->a)\n"
              "    {\n"
              "        g_free(abc);\n"
              "        return 0;\n"
              "    }\n"
              "    return abc;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void function1() {
        // Not found function => assume that the function may deallocate
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "    func(abc);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = g_malloc(sizeof(struct ABC));\n"
              "    abc->a = g_malloc(10);\n"
              "    g_func(abc);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abclist.push_back(abc);\n"
              "    abc->a = malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = g_malloc(sizeof(struct ABC));\n"
              "    abclist.push_back(abc);\n"
              "    abc->a = g_malloc(10);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    // #2848: Taking address in function 'assign'
    void function2() {
        check("void f() {\n"
              "  A a = { 0 };\n"
              "  a.foo = (char *) malloc(10);\n"
              "  assign(&a);\n"
              "}\n", "test.c");
        ASSERT_EQUALS("", errout.str());
        check("void f() {\n"
              "  A a = { 0 };\n"
              "  a.foo = (gchar *) g_malloc(10);\n"
              "  assign(&a);\n"
              "}\n", "test.c");
        ASSERT_EQUALS("", errout.str());
    }

    // #3024: kernel list
    void function3() {
        check("void f() {\n"
              "  struct ABC *abc = kmalloc(100);\n"
              "  abc.a = (char *) kmalloc(10);\n"
              "  list_add_tail(&abc->list, head);\n"
              "}\n", "test.c");
        ASSERT_EQUALS("", errout.str());
    }

    // #3038: deallocating in function
    void function4() {
        check("void a(char *p) { char *x = p; free(x); }\n"
              "void b() {\n"
              "  struct ABC abc;\n"
              "  abc.a = (char *) malloc(10);\n"
              "  a(abc.a);\n"
              "}\n", "test.c");
        ASSERT_EQUALS("", errout.str());
        check("void a(char *p) { gchar *x = p; g_free(x); }\n"
              "void b() {\n"
              "  struct ABC abc;\n"
              "  abc.a = (gchar *) g_malloc(10);\n"
              "  a(abc.a);\n"
              "}\n", "test.c");
        ASSERT_EQUALS("", errout.str());
    }

    void ifelse() {
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    if (x)"
              "    {\n"
              "        abc->a = malloc(10);\n"
              "    }\n"
              "    else\n"
              "    {\n"
              "        free(abc);\n"
              "        return;\n"
              "    }\n"
              "    free(abc->a);\n"
              "    free(abc);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("static void foo()\n"
              "{\n"
              "    struct ABC *abc = g_malloc(sizeof(struct ABC));\n"
              "    if (x)"
              "    {\n"
              "        abc->a = g_malloc(10);\n"
              "    }\n"
              "    else\n"
              "    {\n"
              "        g_free(abc);\n"
              "        return;\n"
              "    }\n"
              "    g_free(abc->a);\n"
              "    g_free(abc);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void linkedlist() {
        // #3904 - false positive when linked list is used
        check("static void foo() {\n"
              "    struct ABC *abc = malloc(sizeof(struct ABC));\n"
              "    abc->next = malloc(sizeof(struct ABC));\n"
              "    abc->next->next = NULL;\n"
              "\n"
              "    while (abc) {\n"
              "        struct ABC *next = abc->next;\n"
              "        free(abc);\n"
              "        abc = next;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("static void foo() {\n"
              "    struct ABC *abc = g_malloc(sizeof(struct ABC));\n"
              "    abc->next = g_malloc(sizeof(struct ABC));\n"
              "    abc->next->next = NULL;\n"
              "\n"
              "    while (abc) {\n"
              "        struct ABC *next = abc->next;\n"
              "        g_free(abc);\n"
              "        abc = next;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void globalvar() {
        check("struct ABC *abc;\n"
              "\n"
              "static void foo()\n"
              "{\n"
              "    abc = malloc(sizeof(struct ABC));\n"
              "    abc->a = malloc(10);\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("struct ABC *abc;\n"
              "\n"
              "static void foo()\n"
              "{\n"
              "    abc = g_malloc(sizeof(struct ABC));\n"
              "    abc->a = g_malloc(10);\n"
              "    return;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    // Ticket #933 Leaks with struct members not detected
    void localvars() {
        // Test error case
        const char code_err[] = "struct A {\n"
                                "    FILE* f;\n"
                                "    char* c;\n"
                                "    void* m;\n"
                                "};\n"
                                "\n"
                                "void func() {\n"
                                "    struct A a;\n"
                                "    a.f = fopen(\"test\", \"r\");\n"
                                "    a.c = new char[12];\n"
                                "    a.m = malloc(12);\n"
                                "}\n";

        check(code_err, "test.cpp");
        ASSERT_EQUALS("", errout.str());
        check(code_err, "test.c");
        ASSERT_EQUALS("[test.c:12]: (error) Memory leak: a.f\n"
                      "[test.c:12]: (error) Memory leak: a.c\n"
                      "[test.c:12]: (error) Memory leak: a.m\n", errout.str());
        const char code_err_glib[] = "struct A {\n"
                                     "    FILE* f;\n"
                                     "    char* c;\n"
                                     "    void* m;\n"
                                     "};\n"
                                     "\n"
                                     "void func() {\n"
                                     "    struct A a;\n"
                                     "    a.f = fopen(\"test\", \"r\");\n"
                                     "    a.c = new char[12];\n"
                                     "    a.m = g_malloc(12);\n"
                                     "}\n";

        check(code_err_glib, "test.cpp");
        ASSERT_EQUALS("", errout.str());
        check(code_err_glib, "test.c");
        TODO_ASSERT_EQUALS("[test.c:12]: (error) Memory leak: a.f\n"
                           "[test.c:12]: (error) Memory leak: a.c\n"
                           "[test.c:12]: (error) Memory leak: a.m\n",
                           "[test.c:12]: (error) Memory leak: a.f\n"
                           "[test.c:12]: (error) Memory leak: a.c\n", errout.str());

        // Test OK case
        const char code_ok[] = "struct A {\n"
                               "    FILE* f;\n"
                               "    char* c;\n"
                               "    void* m;\n"
                               "};\n"
                               "\n"
                               "void func() {\n"
                               "    struct A a;\n"
                               "    a.f = fopen(\"test\", \"r\");\n"
                               "    a.c = new char[12];\n"
                               "    a.m = malloc(12);\n"
                               "    fclose(a.f);\n"
                               "    delete [] a.c;\n"
                               "    free(a.m);\n"
                               "}\n";

        check(code_ok, "test.cpp");
        ASSERT_EQUALS("", errout.str());
        check(code_ok, "test.c");
        ASSERT_EQUALS("", errout.str());
        const char code_ok_glib[] = "struct A {\n"
                                    "    FILE* f;\n"
                                    "    char* c;\n"
                                    "    void* m;\n"
                                    "};\n"
                                    "\n"
                                    "void func() {\n"
                                    "    struct A a;\n"
                                    "    a.f = fopen(\"test\", \"r\");\n"
                                    "    a.c = new char[12];\n"
                                    "    a.m = g_malloc(12);\n"
                                    "    fclose(a.f);\n"
                                    "    delete [] a.c;\n"
                                    "    g_free(a.m);\n"
                                    "}\n";

        check(code_ok_glib, "test.cpp");
        ASSERT_EQUALS("", errout.str());
        check(code_ok_glib, "test.c");
        ASSERT_EQUALS("", errout.str());
    }

    // don't crash
    void trac5030() {
        check("bool bob( char const **column_ptrs ) {\n"
              "unique_ptr<char[]>otherbuffer{new char[otherbufsize+1]};\n"
              "char *const oldbuffer = otherbuffer.get();\n"
              "int const oldbufsize = otherbufsize;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void varid() { // #5201
        check("struct S {\n"
              "  void *state_check_buff;\n"
              "};\n"
              "void f() {\n"
              "  S s;\n"
              "  (s).state_check_buff = (void* )malloc(1);\n"
              "  if (s.state_check_buff == 0)\n"
              "    return;\n"
              "}", /*fname=*/0, /*isCPP=*/false);
        ASSERT_EQUALS("[test.c:9]: (error) Memory leak: s.state_check_buff\n", errout.str());
        check("struct S {\n"
              "  void *state_check_buff;\n"
              "};\n"
              "void f() {\n"
              "  S s;\n"
              "  (s).state_check_buff = (void* )g_malloc(1);\n"
              "  if (s.state_check_buff == 0)\n"
              "    return;\n"
              "}", /*fname=*/0, /*isCPP=*/false);
        TODO_ASSERT_EQUALS("[test.c:9]: (error) Memory leak: s.state_check_buff\n", "", errout.str());
    }

    void varid_2() { // #5315
        check("typedef struct foo { char *realm; } foo;\n"
              "void build_principal() {\n"
              "  foo f;\n"
              "  ((f)->realm) = strdup(realm);\n"
              "  if(f->realm == NULL) {}\n"
              "}", /*fname=*/0, /*isCPP=*/false);
        ASSERT_EQUALS("[test.c:6]: (error) Memory leak: f.realm\n", errout.str());
    }
};


static TestMemleakStructMember testMemleakStructMember;





class TestMemleakNoVar : public TestFixture {
public:
    TestMemleakNoVar() : TestFixture("TestMemleakNoVar") {
    }

private:
    Settings settings;

    void check(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();

        // Check for memory leaks..
        CheckMemoryLeakNoVar checkMemoryLeakNoVar(&tokenizer, &settings, this);
        checkMemoryLeakNoVar.check();
    }

    void run() {
        settings.inconclusive = true;
        settings.addEnabled("warning");

        LOAD_LIB_2(settings.library, "gtk.cfg");

        // Add some test allocation functions to the library.
        // When not run as a unit test, these are read from
        // an XML file (e.g. cfg/posix.cfg).
        int id = 0;
        while (!settings.library.ismemory(++id))
            continue;
        settings.library.setalloc("malloc", id);
        settings.library.setalloc("calloc", id);
        settings.library.setalloc("strdup", id);


        // pass allocated memory to function..
        TEST_CASE(functionParameter);

        // never use leakable resource
        TEST_CASE(missingAssignment);

        // pass allocated memory to function using a smart pointer
        TEST_CASE(smartPointerFunctionParam);
    }

    void functionParameter() {
        // standard function..
        check("void x() {\n"
              "    strcpy(a, strdup(p));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Allocation with strdup, strcpy doesn't release it.\n", errout.str());
        check("void x() {\n"
              "    g_strlcpy(a, g_strdup(p));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Allocation with g_strdup, g_strlcpy doesn't release it.\n", errout.str());

        check("char *x() {\n"
              "    char *ret = strcpy(malloc(10), \"abc\");\n"
              "    return ret;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("gchar *x() {\n"
              "    gchar *ret = g_strlcpy(g_malloc(10), \"abc\");\n"
              "    return ret;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void x() {\n"
              "    free(malloc(10));\n"
              "}");
        ASSERT_EQUALS("", errout.str());
        check("void x() {\n"
              "    g_free(g_malloc(10));\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // user function..
        check("void set_error(const char *msg) {\n"
              "}\n"
              "\n"
              "void x() {\n"
              "    set_error(strdup(p));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Allocation with strdup, set_error doesn't release it.\n", errout.str());
        check("void set_error(const char *msg) {\n"
              "}\n"
              "\n"
              "void x() {\n"
              "    set_error(g_strdup(p));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Allocation with g_strdup, set_error doesn't release it.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    int fd;\n"
              "    fd = mkstemp(strdup(\"/tmp/file.XXXXXXXX\"));\n"
              "    close(fd);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:4]: (error) Allocation with strdup, mkstemp doesn't release it.\n", "", errout.str());

        check("void f()\n"
              "{\n"
              "    if(TRUE || strcmp(strdup(a), b));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Allocation with strdup, strcmp doesn't release it.\n", errout.str());
        check("void f()\n"
              "{\n"
              "    if(TRUE || g_strcmp0(g_strdup(a), b));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Allocation with g_strdup, g_strcmp0 doesn't release it.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    if(!strcmp(strdup(a), b) == 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Allocation with strdup, strcmp doesn't release it.\n", errout.str());
        check("void f()\n"
              "{\n"
              "    if(!g_strcmp0(g_strdup(a), b) == 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Allocation with g_strdup, g_strcmp0 doesn't release it.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    42, strcmp(strdup(a), b);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Allocation with strdup, strcmp doesn't release it.\n", errout.str());
        check("void f()\n"
              "{\n"
              "    42, g_strcmp0(g_strdup(a), b);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Allocation with g_strdup, g_strcmp0 doesn't release it.\n", errout.str());
    }

    void missingAssignment() {
        check("void x()\n"
              "{\n"
              "    malloc(10);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Return value of allocation function malloc is not stored.\n", errout.str());

        check("void x()\n"
              "{\n"
              "    calloc(10);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Return value of allocation function calloc is not stored.\n", errout.str());

        check("void x()\n"
              "{\n"
              "    strdup(\"Test\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Return value of allocation function strdup is not stored.\n", errout.str());

        check("void x()\n"
              "{\n"
              "    (char*) malloc(10);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Return value of allocation function malloc is not stored.\n", errout.str());

        check("void x()\n"
              "{\n"
              "    char* ptr = malloc(10);\n"
              "    foo(ptr);\n"
              "    free(ptr);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("char** x(const char* str) {\n"
              "    char* ptr[] = { malloc(10), malloc(5), strdup(str) };\n"
              "    return ptr;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void x()\n"
              "{\n"
              "    42,malloc(42);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Return value of allocation function malloc is not stored.\n", errout.str());

        check("void *f()\n"
              "{\n"
              "    return malloc(10);\n"
              "}\n"
              "void x()\n"
              "{\n"
              "    f();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Return value of allocation function f is not stored.\n", errout.str());

        check("void x()\n"
              "{\n"
              "    if(!malloc(5)) fail();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Return value of allocation function malloc is not stored.\n", errout.str());

        check("FOO* factory() {\n"
              "    FOO* foo = new (std::nothrow) FOO;\n"
              "    return foo;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void smartPointerFunctionParam() {
        check("void x() {\n"
              "    f(shared_ptr<int>(new int(42)), g());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Unsafe allocation. If g() throws, memory could be leaked. Use make_shared<int>() instead.\n", errout.str());

        check("void x() {\n"
              "    h(12, f(shared_ptr<int>(new int(42)), g()));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Unsafe allocation. If g() throws, memory could be leaked. Use make_shared<int>() instead.\n", errout.str());

        check("void x() {\n"
              "    f(unique_ptr<int>(new int(42)), g());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Unsafe allocation. If g() throws, memory could be leaked. Use make_unique<int>() instead.\n", errout.str());

        check("void x() {\n"
              "    f(g(), shared_ptr<int>(new int(42)));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Unsafe allocation. If g() throws, memory could be leaked. Use make_shared<int>() instead.\n", errout.str());

        check("void x() {\n"
              "    f(g(), unique_ptr<int>(new int(42)));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Unsafe allocation. If g() throws, memory could be leaked. Use make_unique<int>() instead.\n", errout.str());

        check("void x() {\n"
              "    f(shared_ptr<char>(new char), make_unique<int>(32));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Unsafe allocation. If make_unique<int>() throws, memory could be leaked. Use make_shared<char>() instead.\n", errout.str());

        check("void x() {\n"
              "    f(g(124), h(\"test\", 234), shared_ptr<char>(new char));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning, inconclusive) Unsafe allocation. If h() throws, memory could be leaked. Use make_shared<char>() instead.\n", errout.str());

        check("void g(int x) throw() { }\n"
              "void x() {\n"
              "    f(g(124), shared_ptr<char>(new char));\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void __declspec(nothrow) g(int x) { }\n"
              "void x() {\n"
              "    f(g(124), shared_ptr<char>(new char));\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }
};
REGISTER_TEST(TestMemleakNoVar)





class TestMemleakGLib : public TestFixture {
public:
    TestMemleakGLib() : TestFixture("TestMemleakGLib") {
    }

private:
    Settings settings;

    void check(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        // Preprocess...
        Preprocessor preprocessor(&settings, this);
        std::istringstream istrpreproc(code);
        std::map<std::string, std::string> actual;
        preprocessor.preprocess(istrpreproc, actual, "test.c");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(actual[""]);
        tokenizer.tokenize(istr, "test.c");
        tokenizer.simplifyTokenList2();

        // Check for memory leaks..
        CheckMemoryLeakInFunction checkMemoryLeak(&tokenizer, &settings, this);
        checkMemoryLeak.checkReallocUsage();
        checkMemoryLeak.check();
    }

    void run() {
        LOAD_LIB_2(settings.library, "gtk.cfg");

        TEST_CASE(glib1);
    }

    void glib1() {
        check("void f(gchar *_a, gchar *_b) {"
              "  g_return_if_fail(_a);"
              "  gchar *a = g_strdup(_a);"
              "  g_return_if_fail(_b);"
              "  gchar *b = g_strdup(_b);"
              "  g_free(a);"
              "  g_free(b);"
              "}");
        ASSERT_EQUALS("[test.c:1]: (error) Memory leak: a\n", errout.str());
    }
};
static TestMemleakGLib testMemleakGLib;





class TestMemleakWindows : public TestFixture {
public:
    TestMemleakWindows() : TestFixture("TestMemleakWindows") {
    }

private:
    Settings settings;

    void check(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        // Preprocess...
        Preprocessor preprocessor(&settings, this);
        std::istringstream istrpreproc(code);
        std::map<std::string, std::string> actual;
        preprocessor.preprocess(istrpreproc, actual, "test.c");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(actual[""]);
        tokenizer.tokenize(istr, "test.c");
        tokenizer.simplifyTokenList2();

        // Check for memory leaks..
        CheckMemoryLeakInFunction checkMemoryLeak(&tokenizer, &settings, this);
        checkMemoryLeak.checkReallocUsage();
        checkMemoryLeak.check();
    }

    void run() {
        LOAD_LIB_2(settings.library, "windows.cfg");

        TEST_CASE(openfileNoLeak);
    }

    void openfileNoLeak() {
        check("void f() {"
              "  OFSTRUCT OfStr;"
              "  int hFile = OpenFile(\"file\", &OfStr, 0);"
              "}");
        ASSERT_EQUALS("[test.c:1]: (error) Resource leak: hFile\n", errout.str());

        check("void f() {"
              "  OFSTRUCT OfStr;"
              "  int hFile = OpenFile(\"file\", &OfStr, OF_EXIST);"
              "}");
        TODO_ASSERT_EQUALS("", "[test.c:1]: (error) Resource leak: hFile\n", errout.str());
    }
};
static TestMemleakWindows testMemleakWindows;
