// Cppcheck - A tool for static C/C++ code analysis
// Copyright (C) 2007-2019 Cppcheck team.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "clangastdump.h"
#include "settings.h"
#include "tokenize.h"
#include "testsuite.h"


class TestClangAstDump: public TestFixture {
public:
    TestClangAstDump()
        :TestFixture("TestClangAstDump") {
    }


private:
    void run() OVERRIDE {
        TEST_CASE(breakStmt);
        TEST_CASE(continueStmt);
        TEST_CASE(forStmt);
        TEST_CASE(funcdecl1);
        TEST_CASE(funcdecl2);
        TEST_CASE(funcdecl3);
        TEST_CASE(ifelse);
        TEST_CASE(memberExpr);
        TEST_CASE(recordDecl);
        TEST_CASE(typedefDecl1);
        TEST_CASE(typedefDecl2);
        TEST_CASE(typedefDecl3);
        TEST_CASE(vardecl1);
        TEST_CASE(vardecl2);
        TEST_CASE(vardecl3);
        TEST_CASE(whileStmt);
    }

    std::string parse(const char clang[]) {
        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(clang);
        clangastdump::parseClangAstDump(&tokenizer, istr);
        return tokenizer.tokens()->stringifyList(true, false, false, true, false);
    }

    void breakStmt() {
        const char clang[] = "`-FunctionDecl 0x2c31b18 <1.c:1:1, col:34> col:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x2c31c40 <col:12, col:34>\n"
                             "    `-WhileStmt 0x2c31c20 <col:14, col:24>\n"
                             "      |-<<<NULL>>>\n"
                             "      |-IntegerLiteral 0x2c31bf8 <col:21> 'int' 0\n"
                             "      `-BreakStmt 0x3687c18 <col:24>";
        ASSERT_EQUALS("void foo ( ) { while ( 0 ) { break ; } ; }", parse(clang));
    }

    void continueStmt() {
        const char clang[] = "`-FunctionDecl 0x2c31b18 <1.c:1:1, col:34> col:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x2c31c40 <col:12, col:34>\n"
                             "    `-WhileStmt 0x2c31c20 <col:14, col:24>\n"
                             "      |-<<<NULL>>>\n"
                             "      |-IntegerLiteral 0x2c31bf8 <col:21> 'int' 0\n"
                             "      `-ContinueStmt 0x2c31c18 <col:24>";
        ASSERT_EQUALS("void foo ( ) { while ( 0 ) { continue ; } ; }", parse(clang));
    }

    void forStmt() {
        const char clang[] = "`-FunctionDecl 0x2f93ae0 <1.c:1:1, col:56> col:5 main 'int ()'\n"
                             "  `-CompoundStmt 0x2f93dc0 <col:12, col:56>\n"
                             "    |-ForStmt 0x2f93d50 <col:14, col:44>\n"
                             "    | |-DeclStmt 0x2f93c58 <col:19, col:28>\n"
                             "    | | `-VarDecl 0x2f93bd8 <col:19, col:27> col:23 used i 'int' cinit\n"
                             "    | |   `-IntegerLiteral 0x2f93c38 <col:27> 'int' 0\n"
                             "    | |-<<<NULL>>>\n"
                             "    | |-BinaryOperator 0x2f93cd0 <col:30, col:34> 'int' '<'\n"
                             "    | | |-ImplicitCastExpr 0x2f93cb8 <col:30> 'int' <LValueToRValue>\n"
                             "    | | | `-DeclRefExpr 0x2f93c70 <col:30> 'int' lvalue Var 0x2f93bd8 'i' 'int'\n"
                             "    | | `-IntegerLiteral 0x2f93c98 <col:34> 'int' 10\n"
                             "    | |-UnaryOperator 0x2f93d20 <col:38, col:39> 'int' postfix '++'\n"
                             "    | | `-DeclRefExpr 0x2f93cf8 <col:38> 'int' lvalue Var 0x2f93bd8 'i' 'int'\n"
                             "    | `-CompoundStmt 0x2f93d40 <col:43, col:44>\n"
                             "    `-ReturnStmt 0x2f93da8 <col:46, col:53>\n"
                             "      `-IntegerLiteral 0x2f93d88 <col:53> 'int' 0";
        ASSERT_EQUALS("int main ( ) { for ( int i@1 = 0 ; i@1 < 10 ; ++ i@1 ) { } ; return 0 ; }", parse(clang));
    }

    void funcdecl1() {
        const char clang[] = "`-FunctionDecl 0x3122c30 <1.c:1:1, col:22> col:6 foo 'void (int, int)'\n"
                             "  |-ParmVarDecl 0x3122ae0 <col:10, col:14> col:14 x 'int'\n"
                             "  `-ParmVarDecl 0x3122b58 <col:17, col:21> col:21 y 'int'";
        ASSERT_EQUALS("void foo ( int x@1 , int y@2 ) ;", parse(clang));
    }

    void funcdecl2() {
        const char clang[] = "`-FunctionDecl 0x24b2c38 <1.c:1:1, line:4:1> line:1:5 foo 'int (int, int)'\n"
                             "  |-ParmVarDecl 0x24b2ae0 <col:9, col:13> col:13 used x 'int'\n"
                             "  |-ParmVarDecl 0x24b2b58 <col:16, col:20> col:20 used y 'int'\n"
                             "  `-CompoundStmt 0x24b2de8 <line:2:1, line:4:1>\n"
                             "    `-ReturnStmt 0x24b2dd0 <line:3:5, col:16>\n"
                             "      `-BinaryOperator 0x24b2da8 <col:12, col:16> 'int' '/'\n"
                             "        |-ImplicitCastExpr 0x24b2d78 <col:12> 'int' <LValueToRValue>\n"
                             "        | `-DeclRefExpr 0x24b2d28 <col:12> 'int' lvalue ParmVar 0x24b2ae0 'x' 'int'\n"
                             "        `-ImplicitCastExpr 0x24b2d90 <col:16> 'int' <LValueToRValue>\n"
                             "          `-DeclRefExpr 0x24b2d50 <col:16> 'int' lvalue ParmVar 0x24b2b58 'y' 'int'";
        ASSERT_EQUALS("int foo ( int x@1 , int y@2 ) {\n\n"
                      "return x@1 / y@2 ; }", parse(clang));
    }

    void funcdecl3() {
        const char clang[] = "|-FunctionDecl 0x27cb6b8 <line:865:1, col:35> col:12 __overflow 'int (FILE *, int)' extern\n"
                             "| |-ParmVarDecl 0x27cb528 <col:24, col:29> col:30 'FILE *'\n"
                             "| `-ParmVarDecl 0x27cb5a0 <col:32> col:35 'int'";
        ASSERT_EQUALS("int __overflow ( FILE * , int ) ;", parse(clang));
    }

    void ifelse() {
        const char clang[] = "`-FunctionDecl 0x2637ba8 <1.c:1:1, line:4:1> line:1:5 foo 'int (int)'\n"
                             "  |-ParmVarDecl 0x2637ae0 <col:9, col:13> col:13 used x 'int'\n"
                             "  `-CompoundStmt 0x2637d70 <col:16, line:4:1>\n"
                             "    `-IfStmt 0x2637d38 <line:2:5, line:3:11>\n"
                             "      |-<<<NULL>>>\n"
                             "      |-<<<NULL>>>\n"
                             "      |-BinaryOperator 0x2637cf0 <line:2:9, col:13> 'int' '>'\n"
                             "      | |-ImplicitCastExpr 0x2637cd8 <col:9> 'int' <LValueToRValue>\n"
                             "      | | `-DeclRefExpr 0x2637c90 <col:9> 'int' lvalue ParmVar 0x2637ae0 'x' 'int'\n"
                             "      | `-IntegerLiteral 0x2637cb8 <col:13> 'int' 10\n"
                             "      |-CompoundStmt 0x2637d18 <col:17, col:18>\n"
                             "      `-CompoundStmt 0x2637d28 <line:3:10, col:11>";
        ASSERT_EQUALS("int foo ( int x@1 ) {\n"
                      "if ( x@1 > 10 ) { }\n"
                      "else { } ; }", parse(clang));
    }

    void memberExpr() {
        // C code:
        // struct S { int x };
        // int foo(struct S s) { return s.x; }
        const char clang[] = "|-RecordDecl 0x2441a88 <1.c:1:1, col:18> col:8 struct S definition\n"
                             "| `-FieldDecl 0x2441b48 <col:12, col:16> col:16 referenced x 'int'\n"
                             "`-FunctionDecl 0x2441cf8 <line:2:1, col:35> col:5 foo 'int (struct S)'\n"
                             "  |-ParmVarDecl 0x2441be8 <col:9, col:18> col:18 used s 'struct S':'struct S'\n"
                             "  `-CompoundStmt 0x2441e70 <col:21, col:35>\n"
                             "    `-ReturnStmt 0x2441e58 <col:23, col:32>\n"
                             "      `-ImplicitCastExpr 0x2441e40 <col:30, col:32> 'int' <LValueToRValue>\n"
                             "        `-MemberExpr 0x2441e08 <col:30, col:32> 'int' lvalue .x 0x2441b48\n"
                             "          `-DeclRefExpr 0x2441de0 <col:30> 'struct S':'struct S' lvalue ParmVar 0x2441be8 's' 'struct S':'struct S'";
        ASSERT_EQUALS("struct S { int x@1 ; } ;\n"
                      "int foo ( struct S s@2 ) { return s@2 . x@1 ; }",
                      parse(clang));
    }

    void recordDecl() {
        const char clang[] = "`-RecordDecl 0x354eac8 <1.c:1:1, line:4:1> line:1:8 struct S definition\n"
                             "  |-FieldDecl 0x354eb88 <line:2:3, col:7> col:7 x 'int'\n"
                             "  `-FieldDecl 0x354ebe8 <line:3:3, col:7> col:7 y 'int'";
        ASSERT_EQUALS("struct S\n"
                      "{ int x@1 ;\n"
                      "int y@2 ; } ;",
                      parse(clang));
    }

    void typedefDecl1() {
        const char clang[] = "|-TypedefDecl 0x2d60180 <<invalid sloc>> <invalid sloc> implicit __int128_t '__int128'\n"
                             "| `-BuiltinType 0x2d5fe80 '__int128'";
        ASSERT_EQUALS("typedef __int128 __int128_t ;", parse(clang));
    }

    void typedefDecl2() {
        const char clang[] = "|-TypedefDecl 0x2d604a8 <<invalid sloc>> <invalid sloc> implicit __NSConstantString 'struct __NSConstantString_tag'\n"
                             "| `-RecordType 0x2d602c0 'struct __NSConstantString_tag'\n"
                             "|   `-Record 0x2d60238 '__NSConstantString_tag'";
        ASSERT_EQUALS("typedef struct __NSConstantString_tag __NSConstantString ;", parse(clang));
    }

    void typedefDecl3() {
        const char clang[] = "|-TypedefDecl 0x2d60540 <<invalid sloc>> <invalid sloc> implicit __builtin_ms_va_list 'char *'\n"
                             "| `-PointerType 0x2d60500 'char *'\n"
                             "|   `-BuiltinType 0x2d5f980 'char'";
        ASSERT_EQUALS("typedef char * __builtin_ms_va_list ;", parse(clang));
    }

    void vardecl1() {
        const char clang[] = "|-VarDecl 0x32b8aa0 <1.c:1:1, col:9> col:5 used a 'int' cinit\n"
                             "| `-IntegerLiteral 0x32b8b40 <col:9> 'int' 1\n"
                             "`-VarDecl 0x32b8b78 <line:2:1, col:9> col:5 b 'int' cinit\n"
                             "  `-ImplicitCastExpr 0x32b8c00 <col:9> 'int' <LValueToRValue>\n"
                             "    `-DeclRefExpr 0x32b8bd8 <col:9> 'int' lvalue Var 0x32b8aa0 'a' 'int'";

        ASSERT_EQUALS("int a@1 = 1 ;\n"
                      "int b@2 = a@1 ;",
                      parse(clang));
    }

    void vardecl2() {
        const char clang[] = "|-VarDecl 0x3873b50 <1.c:1:1, col:9> col:5 used a 'int [10]'\n"
                             "`-FunctionDecl 0x3873c38 <line:3:1, line:6:1> line:3:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x3873dd0 <line:4:1, line:6:1>\n"
                             "    `-BinaryOperator 0x3873da8 <line:5:3, col:10> 'int' '='\n"
                             "      |-ArraySubscriptExpr 0x3873d60 <col:3, col:6> 'int' lvalue\n"
                             "      | |-ImplicitCastExpr 0x3873d48 <col:3> 'int *' <ArrayToPointerDecay>\n"
                             "      | | `-DeclRefExpr 0x3873cd8 <col:3> 'int [10]' lvalue Var 0x3873b50 'a' 'int [10]'\n"
                             "      | `-IntegerLiteral 0x3873d00 <col:5> 'int' 0\n"
                             "      `-IntegerLiteral 0x3873d88 <col:10> 'int' 0\n";

        ASSERT_EQUALS("int [10] a@1 ;\n" // <- TODO
                      "\n"
                      "void foo ( ) {\n"
                      "\n"
                      "a@1 [ 0 ] = 0 ; }",
                      parse(clang));
    }

    void vardecl3() {
        const char clang[] = "`-VarDecl 0x25a8aa0 <1.c:1:1, col:12> col:12 p 'const int *'";
        ASSERT_EQUALS("const int * p@1 ;", parse(clang));
    }

    void whileStmt() {
        const char clang[] = "`-FunctionDecl 0x3d45b18 <1.c:1:1, line:3:1> line:1:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x3d45c48 <col:12, line:3:1>\n"
                             "    `-WhileStmt 0x3d45c28 <line:2:5, col:14>\n"
                             "      |-<<<NULL>>>\n"
                             "      |-IntegerLiteral 0x3d45bf8 <col:12> 'int' 0\n"
                             "      `-NullStmt 0x3d45c18 <col:14>";
        ASSERT_EQUALS("void foo ( ) {\n"
                      "while ( 0 ) { ; } ; }",
                      parse(clang));
    }
};

REGISTER_TEST(TestClangAstDump)
