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

#include "clangimport.h"
#include "settings.h"
#include "tokenize.h"
#include "testsuite.h"


class TestClangImport: public TestFixture {
public:
    TestClangImport()
        :TestFixture("TestClangImport") {
    }


private:
    void run() OVERRIDE {
        TEST_CASE(breakStmt);
        TEST_CASE(characterLiteral);
        TEST_CASE(class1);
        TEST_CASE(classTemplateDecl1);
        TEST_CASE(classTemplateDecl2);
        TEST_CASE(continueStmt);
        TEST_CASE(cstyleCastExpr);
        TEST_CASE(cxxBoolLiteralExpr);
        TEST_CASE(cxxConstructorDecl);
        TEST_CASE(cxxConstructExpr1);
        TEST_CASE(cxxConstructExpr2);
        TEST_CASE(cxxMemberCall);
        TEST_CASE(cxxMethodDecl);
        TEST_CASE(cxxNullPtrLiteralExpr);
        TEST_CASE(cxxOperatorCallExpr);
        TEST_CASE(cxxRecordDecl1);
        TEST_CASE(cxxStaticCastExpr1);
        TEST_CASE(cxxStaticCastExpr2);
        TEST_CASE(forStmt);
        TEST_CASE(funcdecl1);
        TEST_CASE(funcdecl2);
        TEST_CASE(funcdecl3);
        TEST_CASE(funcdecl4);
        TEST_CASE(functionTemplateDecl1);
        TEST_CASE(functionTemplateDecl2);
        TEST_CASE(ifelse);
        TEST_CASE(memberExpr);
        TEST_CASE(namespaceDecl);
        TEST_CASE(recordDecl);
        TEST_CASE(typedefDecl1);
        TEST_CASE(typedefDecl2);
        TEST_CASE(typedefDecl3);
        TEST_CASE(unaryExprOrTypeTraitExpr);
        TEST_CASE(unaryOperator);
        TEST_CASE(vardecl1);
        TEST_CASE(vardecl2);
        TEST_CASE(vardecl3);
        TEST_CASE(vardecl4);
        TEST_CASE(vardecl5);
        TEST_CASE(whileStmt);
    }

    std::string parse(const char clang[]) {
        Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(clang);
        clangimport::parseClangAstDump(&tokenizer, istr);
        return tokenizer.tokens()->stringifyList(true, false, false, true, false);
    }

    void breakStmt() {
        const char clang[] = "`-FunctionDecl 0x2c31b18 <1.c:1:1, col:34> col:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x2c31c40 <col:12, col:34>\n"
                             "    `-WhileStmt 0x2c31c20 <col:14, col:24>\n"
                             "      |-<<<NULL>>>\n"
                             "      |-IntegerLiteral 0x2c31bf8 <col:21> 'int' 0\n"
                             "      `-BreakStmt 0x3687c18 <col:24>";
        ASSERT_EQUALS("void foo ( ) { while ( 0 ) { break ; } }", parse(clang));
    }

    void characterLiteral() {
        const char clang[] = "`-VarDecl 0x3df8608 <a.cpp:1:1, col:10> col:6 c 'char' cinit\n"
                             "  `-CharacterLiteral 0x3df86a8 <col:10> 'char' 120";
        ASSERT_EQUALS("char c@1 = 'x' ;", parse(clang));
    }

    void class1() {
        const char clang[] = "`-CXXRecordDecl 0x274c638 <a.cpp:1:1, col:25> col:7 class C definition\n"
                             "  |-DefinitionData pass_in_registers empty aggregate standard_layout trivially_copyable pod trivial literal has_constexpr_non_copy_move_ctor can_const_default_init\n"
                             "  | |-DefaultConstructor exists trivial constexpr needs_implicit defaulted_is_constexpr\n"
                             "  | |-CopyConstructor simple trivial has_const_param needs_implicit implicit_has_const_param\n"
                             "  | |-MoveConstructor exists simple trivial needs_implicit\n"
                             "  | |-CopyAssignment trivial has_const_param needs_implicit implicit_has_const_param\n"
                             "  | |-MoveAssignment exists simple trivial needs_implicit\n"
                             "  | `-Destructor simple irrelevant trivial needs_implicit\n"
                             "  |-CXXRecordDecl 0x274c758 <col:1, col:7> col:7 implicit class C\n"
                             "  `-CXXMethodDecl 0x274c870 <col:11, col:23> col:16 foo 'void ()'\n"
                             "    `-CompoundStmt 0x274c930 <col:22, col:23>";
        ASSERT_EQUALS("class C { void foo ( ) { } }", parse(clang));
    }

    void classTemplateDecl1() {
        const char clang[] = "`-ClassTemplateDecl 0x29d1748 <a.cpp:1:1, col:59> col:25 C\n"
                             "  |-TemplateTypeParmDecl 0x29d15f8 <col:10, col:16> col:16 referenced class depth 0 index 0 T\n"
                             "  `-CXXRecordDecl 0x29d16b0 <col:19, col:59> col:25 class C definition\n"
                             "    |-DefinitionData empty aggregate standard_layout trivially_copyable pod trivial literal has_constexpr_non_copy_move_ctor can_const_default_init\n"
                             "    | |-DefaultConstructor exists trivial constexpr needs_implicit defaulted_is_constexpr\n"
                             "    | |-CopyConstructor simple trivial has_const_param needs_implicit implicit_has_const_param\n"
                             "    | |-MoveConstructor exists simple trivial needs_implicit\n"
                             "    | |-CopyAssignment trivial has_const_param needs_implicit implicit_has_const_param\n"
                             "    | |-MoveAssignment exists simple trivial needs_implicit\n"
                             "    | `-Destructor simple irrelevant trivial needs_implicit\n"
                             "    |-CXXRecordDecl 0x29d19b0 <col:19, col:25> col:25 implicit class C\n"
                             "    |-AccessSpecDecl 0x29d1a48 <col:29, col:35> col:29 public\n"
                             "    `-CXXMethodDecl 0x29d1b20 <col:37, col:57> col:39 foo 'T ()'\n"
                             "      `-CompoundStmt 0x29d1c18 <col:45, col:57>\n"
                             "        `-ReturnStmt 0x29d1c00 <col:47, col:54>\n"
                             "          `-IntegerLiteral 0x29d1be0 <col:54> 'int' 0";
        ASSERT_EQUALS("", parse(clang));
    }

    void classTemplateDecl2() {
        const char clang[] = "|-ClassTemplateDecl 0x244e748 <a.cpp:1:1, col:59> col:25 C\n"
                             "| |-TemplateTypeParmDecl 0x244e5f8 <col:10, col:16> col:16 referenced class depth 0 index 0 T\n"
                             "| |-CXXRecordDecl 0x244e6b0 <col:19, col:59> col:25 class C definition\n"
                             "| | |-DefinitionData empty aggregate standard_layout trivially_copyable pod trivial literal has_constexpr_non_copy_move_ctor can_const_default_init\n"
                             "| | | |-DefaultConstructor exists trivial constexpr needs_implicit defaulted_is_constexpr\n"
                             "| | | |-CopyConstructor simple trivial has_const_param needs_implicit implicit_has_const_param\n"
                             "| | | |-MoveConstructor exists simple trivial needs_implicit\n"
                             "| | | |-CopyAssignment trivial has_const_param needs_implicit implicit_has_const_param\n"
                             "| | | |-MoveAssignment exists simple trivial needs_implicit\n"
                             "| | | `-Destructor simple irrelevant trivial needs_implicit\n"
                             "| | |-CXXRecordDecl 0x244e9b0 <col:19, col:25> col:25 implicit class C\n"
                             "| | |-AccessSpecDecl 0x244ea48 <col:29, col:35> col:29 public\n"
                             "| | `-CXXMethodDecl 0x244eb20 <col:37, col:57> col:39 foo 'T ()'\n"
                             "| |   `-CompoundStmt 0x244ec18 <col:45, col:57>\n"
                             "| |     `-ReturnStmt 0x244ec00 <col:47, col:54>\n"
                             "| |       `-IntegerLiteral 0x244ebe0 <col:54> 'int' 0\n"
                             "| `-ClassTemplateSpecializationDecl 0x244ed78 <col:1, col:59> col:25 class C definition\n"
                             "|   |-DefinitionData pass_in_registers empty aggregate standard_layout trivially_copyable pod trivial literal has_constexpr_non_copy_move_ctor can_const_default_init\n"
                             "|   | |-DefaultConstructor exists trivial constexpr defaulted_is_constexpr\n"
                             "|   | |-CopyConstructor simple trivial has_const_param implicit_has_const_param\n"
                             "|   | |-MoveConstructor exists simple trivial\n"
                             "|   | |-CopyAssignment trivial has_const_param needs_implicit implicit_has_const_param\n"
                             "|   | |-MoveAssignment exists simple trivial needs_implicit\n"
                             "|   | `-Destructor simple irrelevant trivial needs_implicit\n"
                             "|   |-TemplateArgument type 'int'\n"
                             "|   |-CXXRecordDecl 0x244eff0 prev 0x244ed78 <col:19, col:25> col:25 implicit class C\n"
                             "|   |-AccessSpecDecl 0x244f088 <col:29, col:35> col:29 public\n"
                             "|   |-CXXMethodDecl 0x244f160 <col:37, col:57> col:39 used foo 'int ()'\n"
                             "|   | `-CompoundStmt 0x247cb40 <col:45, col:57>\n"
                             "|   |   `-ReturnStmt 0x247cb28 <col:47, col:54>\n"
                             "|   |     `-IntegerLiteral 0x244ebe0 <col:54> 'int' 0\n"
                             "|   |-CXXConstructorDecl 0x247c540 <col:25> col:25 implicit used constexpr C 'void () noexcept' inline default trivial\n"
                             "|   | `-CompoundStmt 0x247ca00 <col:25>\n"
                             "|   |-CXXConstructorDecl 0x247c658 <col:25> col:25 implicit constexpr C 'void (const C<int> &)' inline default trivial noexcept-unevaluated 0x247c658\n"
                             "|   | `-ParmVarDecl 0x247c790 <col:25> col:25 'const C<int> &'\n"
                             "|   `-CXXConstructorDecl 0x247c828 <col:25> col:25 implicit constexpr C 'void (C<int> &&)' inline default trivial noexcept-unevaluated 0x247c828\n"
                             "|     `-ParmVarDecl 0x247c960 <col:25> col:25 'C<int> &&'\n";
        ASSERT_EQUALS("class C { int foo ( ) { return 0 ; } }", parse(clang));
    }

    void continueStmt() {
        const char clang[] = "`-FunctionDecl 0x2c31b18 <1.c:1:1, col:34> col:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x2c31c40 <col:12, col:34>\n"
                             "    `-WhileStmt 0x2c31c20 <col:14, col:24>\n"
                             "      |-<<<NULL>>>\n"
                             "      |-IntegerLiteral 0x2c31bf8 <col:21> 'int' 0\n"
                             "      `-ContinueStmt 0x2c31c18 <col:24>";
        ASSERT_EQUALS("void foo ( ) { while ( 0 ) { continue ; } }", parse(clang));
    }

    void cstyleCastExpr() {
        const char clang[] = "`-VarDecl 0x2336aa0 <1.c:1:1, col:14> col:5 x 'int' cinit\n"
                             "  `-CStyleCastExpr 0x2336b70 <col:9, col:14> 'int' <NoOp>\n"
                             "    `-CharacterLiteral 0x2336b40 <col:14> 'int' 97";
        ASSERT_EQUALS("int x@1 = ( int ) 'a' ;", parse(clang));
    }

    void cxxBoolLiteralExpr() {
        const char clang[] = "`-VarDecl 0x3940608 <a.cpp:1:1, col:10> col:6 x 'bool' cinit\n"
                             "  `-CXXBoolLiteralExpr 0x39406a8 <col:10> 'bool' true";
        ASSERT_EQUALS("bool x@1 = true ;", parse(clang));
    }

    void cxxConstructorDecl() {
        const char clang[] = "|-CXXConstructorDecl 0x428e890 <col:11, col:24> col:11 C 'void ()'\n"
                             "| `-CompoundStmt 0x428ea58 <col:15, col:24>\n"
                             "|   `-BinaryOperator 0x428ea30 <col:17, col:21> 'int' lvalue '='\n"
                             "|     |-MemberExpr 0x428e9d8 <col:17> 'int' lvalue ->x 0x428e958\n"
                             "|     | `-CXXThisExpr 0x428e9c0 <col:17> 'C *' this\n"
                             "|     `-IntegerLiteral 0x428ea10 <col:21> 'int' 0\n"
                             "`-FieldDecl 0x428e958 <col:26, col:30> col:30 referenced x 'int'";
        ASSERT_EQUALS("void C ( ) { this . x@1 = 0 ; } int x@1", parse(clang));
    }

    void cxxConstructExpr1() {
        const char clang[] = "`-FunctionDecl 0x2dd7940 <line:2:1, col:30> col:5 f 'Foo (Foo)'\n"
                             "  |-ParmVarDecl 0x2dd7880 <col:7, col:11> col:11 used foo 'Foo'\n"
                             "  `-CompoundStmt 0x2dd80c0 <col:16, col:30>\n"
                             "    `-ReturnStmt 0x2dd80a8 <col:18, col:25>\n"
                             "      `-CXXConstructExpr 0x2dd8070 <col:25> 'Foo' 'void (Foo &&) noexcept'\n"
                             "        `-ImplicitCastExpr 0x2dd7f28 <col:25> 'Foo' xvalue <NoOp>\n"
                             "          `-DeclRefExpr 0x2dd7a28 <col:25> 'Foo' lvalue ParmVar 0x2dd7880 'foo' 'Foo'";
        ASSERT_EQUALS("Foo f ( Foo foo@1 ) { return foo@1 ; }", parse(clang));
    }

    void cxxConstructExpr2() {
        const char clang[] = "`-FunctionDecl 0x3e44180 <1.cpp:2:1, col:30> col:13 f 'std::string ()'\n"
                             "  `-CompoundStmt 0x3e4cb80 <col:17, col:30>\n"
                             "    `-ReturnStmt 0x3e4cb68 <col:19, col:27>\n"
                             "      `-CXXConstructExpr 0x3e4cb38 <col:26, col:27> 'std::string':'std::__cxx11::basic_string<char>' '....' list";
        ASSERT_EQUALS("std::string f ( ) { return std::string ( ) ; }", parse(clang));
    }

    void cxxMemberCall() {
        const char clang[] = "`-FunctionDecl 0x320dc80 <line:2:1, col:33> col:6 bar 'void ()'\n"
                             "  `-CompoundStmt 0x323bb08 <col:12, col:33>\n"
                             "    |-DeclStmt 0x323ba40 <col:14, col:22>\n"
                             "    | `-VarDecl 0x320df28 <col:14, col:21> col:21 used c 'C<int>':'C<int>' callinit\n"
                             "    |   `-CXXConstructExpr 0x323ba10 <col:21> 'C<int>':'C<int>' 'void () noexcept'\n"
                             "    `-CXXMemberCallExpr 0x323bab8 <col:24, col:30> 'int':'int'\n"
                             "      `-MemberExpr 0x323ba80 <col:24, col:26> '<bound member function type>' .foo 0x320e160\n"
                             "        `-DeclRefExpr 0x323ba58 <col:24> 'C<int>':'C<int>' lvalue Var 0x320df28 'c' 'C<int>':'C<int>'";
        ASSERT_EQUALS("void bar ( ) { C<int> c@1 ; c@1 . foo ( ) ; }", parse(clang));
    }

    void cxxMethodDecl() {
        const char clang[] = "|-CXXMethodDecl 0x55c786f5ad60 <line:56:5, col:179> col:10 analyzeFile '_Bool (const std::string &, const std::string &, const std::string &, unsigned long long, std::list<ErrorLogger::ErrorMessage> *)'\n"
                             "| |-ParmVarDecl 0x55c786f5a4c8 <col:22, col:41> col:41 buildDir 'const std::string &'\n"
                             "| |-ParmVarDecl 0x55c786f5a580 <col:51, col:70> col:70 sourcefile 'const std::string &'\n"
                             "| |-ParmVarDecl 0x55c786f5a638 <col:82, col:101> col:101 cfg 'const std::string &'\n"
                             "| |-ParmVarDecl 0x55c786f5a6a8 <col:106, col:125> col:125 checksum 'unsigned long long'\n"
                             "| |-ParmVarDecl 0x55c786f5ac00 <col:135, col:173> col:173 errors 'std::list<ErrorLogger::ErrorMessage> *'\n"
                             "  `-CompoundStmt 0x0 <>";
        ASSERT_EQUALS("_Bool analyzeFile ( const std::string & buildDir@1 , const std::string & sourcefile@2 , const std::string & cfg@3 , unsigned long long checksum@4 , std::list<ErrorLogger::ErrorMessage> * errors@5 ) { }", parse(clang));
    }


    void cxxNullPtrLiteralExpr() {
        const char clang[] = "`-VarDecl 0x2a7d650 <1.cpp:1:1, col:17> col:13 p 'const char *' cinit\n"
                             "  `-ImplicitCastExpr 0x2a7d708 <col:17> 'const char *' <NullToPointer>\n"
                             "    `-CXXNullPtrLiteralExpr 0x2a7d6f0 <col:17> 'nullptr_t'";
        ASSERT_EQUALS("const char * p@1 = nullptr ;", parse(clang));
    }

    void cxxOperatorCallExpr() {
        const char clang[] = "`-FunctionDecl 0x3c099f0 <line:2:1, col:24> col:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x3c37308 <col:12, col:24>\n"
                             "    |-DeclStmt 0x3c0a060 <col:14, col:17>\n"
                             "    | `-VarDecl 0x3c09ae0 <col:14, col:16> col:16 used c 'C' callinit\n"
                             "    |   `-CXXConstructExpr 0x3c0a030 <col:16> 'C' 'void () noexcept'\n"
                             "    `-CXXOperatorCallExpr 0x3c372c0 <col:19, col:21> 'void'\n"
                             "      |-ImplicitCastExpr 0x3c372a8 <col:20> 'void (*)(int)' <FunctionToPointerDecay>\n"
                             "      | `-DeclRefExpr 0x3c37250 <col:20> 'void (int)' lvalue CXXMethod 0x3c098c0 'operator=' 'void (int)'\n"
                             "      |-DeclRefExpr 0x3c0a078 <col:19> 'C' lvalue Var 0x3c09ae0 'c' 'C'\n"
                             "      `-IntegerLiteral 0x3c0a0a0 <col:21> 'int' 4";
        ASSERT_EQUALS("void foo ( ) { C c@1 ; c@1 . operator= ( 4 ) ; }", parse(clang));
    }

    void cxxRecordDecl1() {
        const char clang[] = "`-CXXRecordDecl 0x34cc5f8 <1.cpp:2:1, col:7> col:7 class Foo";
        ASSERT_EQUALS("", parse(clang));
    }

    void cxxStaticCastExpr1() {
        const char clang[] = "`-VarDecl 0x2e0e650 <a.cpp:2:1, col:27> col:5 a 'int' cinit\n"
                             "  `-CXXStaticCastExpr 0x2e0e728 <col:9, col:27> 'int' static_cast<int> <NoOp>\n"
                             "    `-IntegerLiteral 0x2e0e6f0 <col:26> 'int' 0";
        ASSERT_EQUALS("int a@1 = static_cast<int> ( 0 ) ;", parse(clang));
    }

    void cxxStaticCastExpr2() {
        const char clang[] = "`-VarDecl 0x2e0e650 <a.cpp:2:1, col:27> col:5 a 'int' cinit\n"
                             "  `-CXXStaticCastExpr 0x3e453e8 <col:12> 'std::_Rb_tree_iterator<std::pair<const std::__cxx11::basic_string<char>, Library::AllocFunc> >' xvalue static_cast<struct std::_Rb_tree_iterator<struct std::pair<const class std::__cxx11::basic_string<char>, struct Library::AllocFunc> > &&> <NoOp>\n"
                             "    `-DeclRefExpr 0x3e453b0 <col:12> 'std::_Rb_tree_iterator<std::pair<const std::__cxx11::basic_string<char>, Library::AllocFunc> >' lvalue ParmVar 0x3e45250 '' 'std::_Rb_tree_iterator<std::pair<const std::__cxx11::basic_string<char>, Library::AllocFunc> > &&'";
        ASSERT_EQUALS("int a@1 = static_cast<structstd::_Rb_tree_iterator<structstd::pair<constclassstd::__cxx11::basic_string<char>,structLibrary::AllocFunc>>&&> ( <NoName> ) ;", parse(clang));
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
        ASSERT_EQUALS("int main ( ) { for ( int i@1 = 0 ; i@1 < 10 ; ++ i@1 ) { } return 0 ; }", parse(clang));
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

    void funcdecl4() {
        const char clang[] = "|-FunctionDecl 0x272bb60 <line:658:15> col:15 implicit fwrite 'unsigned long (const void *, unsigned long, unsigned long, FILE *)' extern\n"
                             "| |-ParmVarDecl 0x272cc40 <<invalid sloc>> <invalid sloc> 'const void *'\n"
                             "| |-ParmVarDecl 0x272cca0 <<invalid sloc>> <invalid sloc> 'unsigned long'\n"
                             "| |-ParmVarDecl 0x272cd00 <<invalid sloc>> <invalid sloc> 'unsigned long'\n"
                             "| `-ParmVarDecl 0x272cd60 <<invalid sloc>> <invalid sloc> 'FILE *'";
        ASSERT_EQUALS("unsigned long fwrite ( const void * , unsigned long , unsigned long , FILE * ) ;", parse(clang));
    }

    void functionTemplateDecl1() {
        const char clang[] = "`-FunctionTemplateDecl 0x3242860 <a.cpp:1:1, col:46> col:21 foo";
        ASSERT_EQUALS("", parse(clang));
    }

    void functionTemplateDecl2() {
        const char clang[] = "|-FunctionTemplateDecl 0x333a860 <a.cpp:1:1, col:46> col:21 foo\n"
                             "| |-TemplateTypeParmDecl 0x333a5f8 <col:10, col:16> col:16 referenced class depth 0 index 0 T\n"
                             "| |-FunctionDecl 0x333a7c0 <col:19, col:46> col:21 foo 'T (T)'\n"
                             "| | |-ParmVarDecl 0x333a6c0 <col:25, col:27> col:27 referenced t 'T'\n"
                             "| | `-CompoundStmt 0x333a980 <col:30, col:46>\n"
                             "| |   `-ReturnStmt 0x333a968 <col:32, col:43>\n"
                             "| |     `-BinaryOperator 0x333a940 <col:39, col:43> '<dependent type>' '+'\n"
                             "| |       |-DeclRefExpr 0x333a8f8 <col:39> 'T' lvalue ParmVar 0x333a6c0 't' 'T'\n"
                             "| |       `-IntegerLiteral 0x333a920 <col:43> 'int' 1\n"
                             "| `-FunctionDecl 0x333ae00 <col:19, col:46> col:21 used foo 'int (int)'\n"
                             "|   |-TemplateArgument type 'int'\n"
                             "|   |-ParmVarDecl 0x333ad00 <col:25, col:27> col:27 used t 'int':'int'\n"
                             "|   `-CompoundStmt 0x333b0a8 <col:30, col:46>\n"
                             "|     `-ReturnStmt 0x333b090 <col:32, col:43>\n"
                             "|       `-BinaryOperator 0x333b068 <col:39, col:43> 'int' '+'\n"
                             "|         |-ImplicitCastExpr 0x333b050 <col:39> 'int':'int' <LValueToRValue>\n"
                             "|         | `-DeclRefExpr 0x333b028 <col:39> 'int':'int' lvalue ParmVar 0x333ad00 't' 'int':'int'\n"
                             "|         `-IntegerLiteral 0x333a920 <col:43> 'int' 1\n"
                             "`-FunctionDecl 0x333a9f8 <line:2:1, col:22> col:1 invalid bar 'int ()'\n"
                             "  `-CompoundStmt 0x333b010 <col:7, col:22>\n"
                             "    `-CallExpr 0x333afe0 <col:9, col:19> 'int':'int'\n"
                             "      |-ImplicitCastExpr 0x333afc8 <col:9, col:16> 'int (*)(int)' <FunctionToPointerDecay>\n"
                             "      | `-DeclRefExpr 0x333af00 <col:9, col:16> 'int (int)' lvalue Function 0x333ae00 'foo' 'int (int)' (FunctionTemplate 0x333a860 'foo')\n"
                             "      `-IntegerLiteral 0x333ab48 <col:18> 'int' 1";
        ASSERT_EQUALS("int foo<int> ( int t@1 ) { return t@1 + 1 ; }\n"
                      "int bar ( ) { foo ( 1 ) ; }", parse(clang));
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
                      "else { } }", parse(clang));
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

    void namespaceDecl() {
        const char clang[] = "`-NamespaceDecl 0x2e5f658 <hello.cpp:1:1, col:24> col:11 x\n"
                             "  `-VarDecl 0x2e5f6d8 <col:15, col:19> col:19 var 'int'";
        ASSERT_EQUALS("namespace x { int var@1 ; }",
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

    void unaryExprOrTypeTraitExpr() {
        const char clang[] = "`-VarDecl 0x24cc610 <a.cpp:1:1, col:19> col:5 x 'int' cinit\n"
                             "  `-ImplicitCastExpr 0x24cc6e8 <col:9, col:19> 'int' <IntegralCast>\n"
                             "    `-UnaryExprOrTypeTraitExpr 0x24cc6c8 <col:9, col:19> 'unsigned long' sizeof 'int'\n";
        ASSERT_EQUALS("int x@1 = sizeof ( int ) ;", parse(clang));
    }

    void unaryOperator() {
        const char clang[] = "`-FunctionDecl 0x2dd9748 <1.cpp:2:1, col:44> col:5 foo 'int (int *)'\n"
                             "  |-ParmVarDecl 0x2dd9680 <col:14, col:19> col:19 used p 'int *'\n"
                             "  `-CompoundStmt 0x2dd9908 <col:22, col:44>\n"
                             "    `-ReturnStmt 0x2dd98f0 <col:24, col:41>\n"
                             "      `-BinaryOperator 0x2dd98c8 <col:31, col:41> 'int' '/'\n"
                             "        |-IntegerLiteral 0x2dd9830 <col:31> 'int' 100000\n"
                             "        `-ImplicitCastExpr 0x2dd98b0 <col:40, col:41> 'int' <LValueToRValue>\n"
                             "          `-UnaryOperator 0x2dd9890 <col:40, col:41> 'int' lvalue prefix '*' cannot overflow\n"
                             "            `-ImplicitCastExpr 0x2dd9878 <col:41> 'int *' <LValueToRValue>\n"
                             "              `-DeclRefExpr 0x2dd9850 <col:41> 'int *' lvalue ParmVar 0x2dd9680 'p' 'int *'";
        ASSERT_EQUALS("int foo ( int * p@1 ) { return 100000 / * p@1 ; }", parse(clang));
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

    void vardecl4() {
        const char clang[] = "|-VarDecl 0x23d6c78 <line:137:1, col:14> col:14 stdin 'FILE *' extern";
        ASSERT_EQUALS("FILE * stdin@1 ;", parse(clang));
    }

    void vardecl5() {
        const char clang[] = "|-VarDecl 0x2e31fc0 <line:27:1, col:38> col:26 sys_errlist 'const char *const []' extern";
        ASSERT_EQUALS("const char *const [] sys_errlist@1 ;", parse(clang));
    }

    void whileStmt() {
        const char clang[] = "`-FunctionDecl 0x3d45b18 <1.c:1:1, line:3:1> line:1:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x3d45c48 <col:12, line:3:1>\n"
                             "    `-WhileStmt 0x3d45c28 <line:2:5, col:14>\n"
                             "      |-<<<NULL>>>\n"
                             "      |-IntegerLiteral 0x3d45bf8 <col:12> 'int' 0\n"
                             "      `-NullStmt 0x3d45c18 <col:14>";
        ASSERT_EQUALS("void foo ( ) {\n"
                      "while ( 0 ) { ; } }",
                      parse(clang));
    }
};

REGISTER_TEST(TestClangImport)
