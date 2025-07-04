// Cppcheck - A tool for static C/C++ code analysis
// Copyright (C) 2007-2024 Cppcheck team.
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
#include "fixture.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"

#include <cstdint>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <utility>
#include <vector>


class TestClangImport : public TestFixture {
public:
    TestClangImport()
        : TestFixture("TestClangImport") {}


private:
    void run() override {
        TEST_CASE(breakStmt);
        TEST_CASE(callExpr);
        TEST_CASE(caseStmt1);
        TEST_CASE(characterLiteral);
        TEST_CASE(class1);
        TEST_CASE(classTemplateDecl1);
        TEST_CASE(conditionalExpr);
        TEST_CASE(compoundAssignOperator);
        TEST_CASE(continueStmt);
        TEST_CASE(cstyleCastExpr);
        TEST_CASE(cxxBoolLiteralExpr);
        TEST_CASE(cxxConstructorDecl1);
        TEST_CASE(cxxConstructorDecl2);
        TEST_CASE(cxxConstructExpr1);
        TEST_CASE(cxxConstructExpr2);
        TEST_CASE(cxxConstructExpr3);
        /*
           TEST_CASE(cxxDeleteExpr);
           TEST_CASE(cxxDestructorDecl);
         */
        TEST_CASE(cxxForRangeStmt1);
        /*
           TEST_CASE(cxxFunctionalCastExpr);
           TEST_CASE(cxxMemberCall);
           TEST_CASE(cxxMethodDecl1);
           TEST_CASE(cxxMethodDecl2);
           TEST_CASE(cxxMethodDecl3);
           TEST_CASE(cxxMethodDecl4);
           TEST_CASE(cxxNewExpr1);
           TEST_CASE(cxxNewExpr2);
           TEST_CASE(cxxNullPtrLiteralExpr);
           TEST_CASE(cxxOperatorCallExpr);
           TEST_CASE(cxxRecordDecl1);
           TEST_CASE(cxxRecordDecl2);
           TEST_CASE(cxxRecordDeclDerived);
           TEST_CASE(cxxStaticCastExpr1);
           TEST_CASE(cxxStaticCastExpr2);
           TEST_CASE(cxxStaticCastExpr3);
           TEST_CASE(cxxStdInitializerListExpr);
           TEST_CASE(cxxThrowExpr);
         */
        TEST_CASE(defaultStmt);
        TEST_CASE(doStmt);
        TEST_CASE(enumDecl1);
        TEST_CASE(enumDecl2);
        TEST_CASE(enumDecl4);
        TEST_CASE(floatLiteral);
        TEST_CASE(forStmt);
        TEST_CASE(funcdecl1);
        TEST_CASE(funcdecl2);
        TEST_CASE(funcdecl3);
        TEST_CASE(funcdecl5);
/*
        TEST_CASE(functionTemplateDecl1);
        TEST_CASE(functionTemplateDecl2);
        TEST_CASE(initListExpr);
 */
        TEST_CASE(ifelse);
        TEST_CASE(ifStmt);
        TEST_CASE(labelStmt);
        TEST_CASE(memberExpr);
        TEST_CASE(namespaceDecl1);
        TEST_CASE(namespaceDecl2);
        TEST_CASE(recordDecl1);
        TEST_CASE(unaryExprOrTypeTraitExpr1);
        TEST_CASE(unaryExprOrTypeTraitExpr2);
        TEST_CASE(unaryOperator);
        TEST_CASE(vardecl1);
        TEST_CASE(vardecl2);
        TEST_CASE(vardecl3);
        TEST_CASE(vardecl4);
        TEST_CASE(vardecl6);
        TEST_CASE(vardecl7);
        TEST_CASE(whileStmt1);
        TEST_CASE(whileStmt2);

        TEST_CASE(tokenIndex);
        //TEST_CASE(symbolDatabaseEnum1);
        TEST_CASE(symbolDatabaseFunction1);
        TEST_CASE(symbolDatabaseFunction2);
        //TEST_CASE(symbolDatabaseFunction3);
        //TEST_CASE(symbolDatabaseFunctionConst);
        //TEST_CASE(symbolDatabaseVariableRef);
        //TEST_CASE(symbolDatabaseVariableRRef);
        //TEST_CASE(symbolDatabaseVariablePointerRef);
        TEST_CASE(symbolDatabaseNodeType1);
        TEST_CASE(symbolDatabaseForVariable);

        TEST_CASE(valueFlow1);
        TEST_CASE(valueFlow2);

        //TEST_CASE(valueType1);
        TEST_CASE(valueType2);
    }

    std::string parse(const std::string& json) {
        const Settings settings = settingsBuilder().clang().build();
        TokenList tokenlist{settings, Standards::Language::CPP};
        Tokenizer tokenizer(std::move(tokenlist), *this);
        clangimport::parseClangAstDump(tokenizer, json);
        if (!tokenizer.tokens()) {
            return std::string();
        }
        return tokenizer.tokens()->stringifyList(true, false, false, false, false);
    }

    void breakStmt() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "f", "type": {"qualType": "void ()"}, "inner": [
            {"kind": "CompoundStmt", "id": "0x3", "inner": [
            {"kind": "WhileStmt", "id": "0x4", "inner": [
            {"kind": "IntegerLiteral", "id": "0x5", "value": "1", "type": {"qualType": "int"}}, 
            {"kind": "CompoundStmt", "id": "0x6", "inner": [
            {"kind": "BreakStmt", "id": "0x7"}]}]}]}]}]})C";

        ASSERT_EQUALS("void f ( ) { while ( 1 ) { break ; } }", parse(clang_json));
    }

    void callExpr() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "f1", "type": {"qualType": "void ()"}}, 
            {"kind": "FunctionDecl", "id": "0x3", "name": "f2", "type": {"qualType": "void ()"}, "inner": [
            {"kind": "CompoundStmt", "id": "0x4", "inner": [
            {"kind": "CallExpr", "id": "0x5", "type": {"qualType": "void"}, "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x6", "type": {"qualType": "void (*)()"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0x7", "type": {"qualType": "void ()"}, "referencedDecl": {"name": "f1", "id": "0x2"}}]}]}]}]}]})C";
        ASSERT_EQUALS("void f1 ( ) ; void f2 ( ) { f1 ( ) ; }", parse(clang_json));
    }

    void caseStmt1() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "foo", "type": {"qualType": "void (int)"}, "inner": [
            {"kind": "ParmVarDecl", "id": "0x3", "name": "x", "type": {"qualType": "int"}}, 
            {"kind": "CompoundStmt", "id": "0x4", "inner": [
            {"kind": "SwitchStmt", "id": "0x5", "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x6", "type": {"qualType": "int"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0x7", "type": {"qualType": "int"}, "referencedDecl": {"name": "x", "id": "0x3"}}]}, 
            {"kind": "CompoundStmt", "id": "0x8", "inner": [
            {"kind": "CaseStmt", "id": "0x9", "inner": [
            {"kind": "ConstantExpr", "id": "0xa", "type": {"qualType": "int"}, "inner": [
            {"kind": "IntegerLiteral", "id": "0xb", "value": "16", "type": {"qualType": "int"}}]}, 
            {"kind": "CaseStmt", "id": "0xc", "inner": [
            {"kind": "ConstantExpr", "id": "0xd", "type": {"qualType": "int"}, "inner": [
            {"kind": "IntegerLiteral", "id": "0xe", "value": "32", "type": {"qualType": "int"}}]}, 
            {"kind": "BinaryOperator", "id": "0xf", "opcode": "=", "type": {"qualType": "int"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0x10", "type": {"qualType": "int"}, "referencedDecl": {"name": "x", "id": "0x3"}}, 
            {"kind": "IntegerLiteral", "id": "0x11", "value": "123", "type": {"qualType": "int"}}]}]}]}, 
            {"kind": "BreakStmt", "id": "0x12"}]}]}]}]}]})C";
        ASSERT_EQUALS("void foo ( int x@1 ) { switch ( x@1 ) { case 16 : case 32 : x@1 = 123 ; break ; } }", parse(clang_json));
    }

    void characterLiteral() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "VarDecl", "id": "0x2", "name": "x", "type": {"qualType": "char"}, "init": "c", "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x3", "type": {"qualType": "char"}, "inner": [
            {"kind": "CharacterLiteral", "id": "0x4", "value": 120, "type": {"qualType": "int"}}]}]}]})C";

        ASSERT_EQUALS("char x@1 = 'x' ;", parse(clang_json));
    }

    void class1() {
        // This json dump was generated by tools/testclangimport.py
        // code: class C { void foo ( ) { } } ;
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "CXXRecordDecl", "id": "0x2", "name": "C", "completeDefinition": true, "tagUsed": "class", "inner": [
            {"kind": "CXXRecordDecl", "id": "0x3", "name": "C", "tagUsed": "class"}, 
            {"kind": "CXXMethodDecl", "id": "0x4", "name": "foo", "type": {"qualType": "void ()"}, "inner": [
            {"kind": "CompoundStmt", "id": "0x5"}]}]}]})C";
        ASSERT_EQUALS("class C { void foo ( ) { } } ;", parse(clang_json));
    }

    void classTemplateDecl1() {
        // This json dump was generated by tools/testclangimport.py
        // code: template<class T> class C { T foo ( T t ) { return t + t; } } ; C<int> c;
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "ClassTemplateDecl", "name": "C", "id": "0x2", "inner": [
            {"kind": "TemplateTypeParmDecl", "name": "T", "tagUsed": "class", "id": "0x3"}, 
            {"kind": "CXXRecordDecl", "name": "C", "completeDefinition": true, "tagUsed": "class", "id": "0x4", "inner": [
            {"kind": "CXXRecordDecl", "name": "C", "tagUsed": "class", "id": "0x5"}, 
            {"kind": "CXXMethodDecl", "name": "foo", "id": "0x6", "type": {"qualType": "T (T)"}, "inner": [
            {"kind": "ParmVarDecl", "name": "t", "id": "0x7", "type": {"qualType": "T"}}, 
            {"kind": "CompoundStmt", "id": "0x8", "inner": [
            {"kind": "ReturnStmt", "id": "0x9", "inner": [
            {"kind": "BinaryOperator", "opcode": "+", "id": "0xa", "type": {"qualType": "<dependent type>"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0xb", "type": {"qualType": "T"}, "referencedDecl": {"name": "t", "id": "0x7"}}, 
            {"kind": "DeclRefExpr", "id": "0xc", "type": {"qualType": "T"}, "referencedDecl": {"name": "t", "id": "0x7"}}]}]}]}]}]}, 
            {"kind": "ClassTemplateSpecializationDecl", "name": "C", "completeDefinition": true, "tagUsed": "class", "id": "0xd", "inner": [
            {"kind": "TemplateArgument", "type": {"qualType": "int"}, "inner": [
            {"kind": "BuiltinType", "id": "0xe", "type": {"qualType": "int"}}]}, 
            {"kind": "CXXRecordDecl", "name": "C", "tagUsed": "class", "id": "0xf"}, 
            {"kind": "CXXMethodDecl", "name": "foo", "id": "0x10", "type": {"qualType": "int (int)"}, "inner": [
            {"kind": "ParmVarDecl", "name": "t", "id": "0x11", "type": {"qualType": "int"}}]}, 
            {"kind": "CXXConstructorDecl", "name": "C", "inline": true, "id": "0x12", "type": {"qualType": "void () noexcept"}, "inner": [
            {"kind": "CompoundStmt", "id": "0x13"}]}, 
            {"kind": "CXXConstructorDecl", "name": "C", "inline": true, "id": "0x14", "type": {"qualType": "void (const C<int> &)"}, "inner": [
            {"kind": "ParmVarDecl", "id": "0x15", "type": {"qualType": "const C<int> &"}}]}, 
            {"kind": "CXXConstructorDecl", "name": "C", "inline": true, "id": "0x16", "type": {"qualType": "void (C<int> &&)"}, "inner": [
            {"kind": "ParmVarDecl", "id": "0x17", "type": {"qualType": "C<int> &&"}}]}]}]}, 
            {"kind": "VarDecl", "name": "c", "init": "call", "id": "0x18", "type": {"qualType": "C<int>"}, "inner": [
            {"kind": "CXXConstructExpr", "id": "0x19", "type": {"qualType": "C<int>"}}]}]})C";
        ASSERT_EQUALS("class C<int> { int foo ( int t@1 ) ; C ( ) { } C ( const C<int> & ) ; C ( C<int> && ) ; } ; C<int> c@2 ;", parse(clang_json));
    }

    void conditionalExpr() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "VarDecl", "id": "0x2", "name": "x", "type": {"qualType": "int"}, "init": "c", "inner": [
            {"kind": "ConditionalOperator", "id": "0x3", "type": {"qualType": "int"}, "inner": [
            {"kind": "IntegerLiteral", "id": "0x4", "value": "1", "type": {"qualType": "int"}}, 
            {"kind": "IntegerLiteral", "id": "0x5", "value": "2", "type": {"qualType": "int"}}, 
            {"kind": "IntegerLiteral", "id": "0x6", "value": "3", "type": {"qualType": "int"}}]}]}]})C";
        ASSERT_EQUALS("int x@1 = 1 ? 2 : 3 ;", parse(clang_json));
    }

    void compoundAssignOperator() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "f", "type": {"qualType": "void ()"}, "inner": [
            {"kind": "CompoundStmt", "id": "0x3", "inner": [
            {"kind": "DeclStmt", "id": "0x4", "inner": [
            {"kind": "VarDecl", "id": "0x5", "name": "x", "type": {"qualType": "int"}, "init": "c", "inner": [
            {"kind": "IntegerLiteral", "id": "0x6", "value": "1", "type": {"qualType": "int"}}]}]}, 
            {"kind": "CompoundAssignOperator", "id": "0x7", "opcode": "+=", "type": {"qualType": "int"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0x8", "type": {"qualType": "int"}, "referencedDecl": {"name": "x", "id": "0x5"}}, 
            {"kind": "IntegerLiteral", "id": "0x9", "value": "1", "type": {"qualType": "int"}}]}]}]}]})C";
        ASSERT_EQUALS("void f ( ) { int x@1 = 1 ; x@1 += 1 ; }", parse(clang_json));
    }

    void continueStmt() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "foo", "type": {"qualType": "void ()"}, "inner": [
            {"kind": "CompoundStmt", "id": "0x3", "inner": [
            {"kind": "WhileStmt", "id": "0x4", "inner": [
            {"kind": "IntegerLiteral", "id": "0x5", "value": "0", "type": {"qualType": "int"}}, 
            {"kind": "CompoundStmt", "id": "0x6", "inner": [
            {"kind": "ContinueStmt", "id": "0x7"}]}]}]}]}]})C";
        ASSERT_EQUALS("void foo ( ) { while ( 0 ) { continue ; } }", parse(clang_json));
    }

    void cstyleCastExpr() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "VarDecl", "id": "0x2", "name": "x", "type": {"qualType": "int"}, "init": "c", "inner": [
            {"kind": "CStyleCastExpr", "id": "0x3", "type": {"qualType": "int"}, "inner": [
            {"kind": "CharacterLiteral", "id": "0x4", "value": 97, "type": {"qualType": "int"}}]}]}]})C";
        ASSERT_EQUALS("int x@1 = ( int ) 'a' ;", parse(clang_json));
    }


    void cxxBoolLiteralExpr() {
        // This json dump was generated by tools/testclangimport.py
        // code: bool x = true ;
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "VarDecl", "name": "x", "init": "c", "id": "0x2", "type": {"qualType": "bool"}, "inner": [
            {"kind": "CXXBoolLiteralExpr", "value": true, "id": "0x3", "type": {"qualType": "bool"}}]}]})C";

        ASSERT_EQUALS("bool x@1 = true ;", parse(clang_json));
    }

    void cxxConstructorDecl1() {
        // This json dump was generated by tools/testclangimport.py
        // code: class C {   C() { x = 0; }   int x; };
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "CXXRecordDecl", "name": "C", "completeDefinition": true, "tagUsed": "class", "id": "0x2", "inner": [
            {"kind": "CXXRecordDecl", "name": "C", "tagUsed": "class", "id": "0x3"}, 
            {"kind": "CXXConstructorDecl", "name": "C", "id": "0x4", "type": {"qualType": "void ()"}, "inner": [
            {"kind": "CompoundStmt", "id": "0x5", "inner": [
            {"kind": "BinaryOperator", "opcode": "=", "id": "0x6", "type": {"qualType": "int"}, "inner": [
            {"kind": "MemberExpr", "name": "x", "id": "0x7", "referencedMemberDecl": "0x8", "type": {"qualType": "int"}, "inner": [
            {"kind": "CXXThisExpr", "id": "0x9", "type": {"qualType": "C *"}}]}, 
            {"kind": "IntegerLiteral", "value": "0", "id": "0xa", "type": {"qualType": "int"}}]}]}]}, 
            {"kind": "FieldDecl", "name": "x", "id": "0x8", "type": {"qualType": "int"}}]}]})C";

        ASSERT_EQUALS("class C { C ( ) { this . x@1 = 0 ; } int x@1 ; } ;", parse(clang_json));
    }

    void cxxConstructorDecl2() {
        // This json dump was generated by tools/testclangimport.py
        // code: class C {   C(C&& other) = default; };
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "CXXRecordDecl", "name": "C", "completeDefinition": true, "tagUsed": "class", "id": "0x2", "inner": [
            {"kind": "CXXRecordDecl", "name": "C", "tagUsed": "class", "id": "0x3"}, 
            {"kind": "CXXConstructorDecl", "name": "C", "explicitlyDefaulted": "default", "id": "0x4", "type": {"qualType": "void (C &&)"}, "inner": [
            {"kind": "ParmVarDecl", "name": "other", "id": "0x5", "type": {"qualType": "C &&"}}]}, 
            {"kind": "CXXConstructorDecl", "name": "C", "explicitlyDefaulted": "deleted", "inline": true, "id": "0x6", "type": {"qualType": "void (const C &)"}, "inner": [
            {"kind": "ParmVarDecl", "id": "0x7", "type": {"qualType": "const C &"}}]}, 
            {"kind": "CXXMethodDecl", "name": "operator=", "explicitlyDefaulted": "deleted", "inline": true, "id": "0x8", "type": {"qualType": "C &(const C &)"}, "inner": [
            {"kind": "ParmVarDecl", "id": "0x9", "type": {"qualType": "const C &"}}]}]}]})C";

        ASSERT_EQUALS("class C { C ( C && other@1 ) = default ; C ( const C & ) ; inline C & operator= ( const C & ) ; } ;", parse(clang_json));
    }

    void cxxConstructExpr1() {
        // This json dump was generated by tools/testclangimport.py
        // code: struct Foo {}; Foo foo(Foo foo) { return foo; }
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "CXXRecordDecl", "name": "Foo", "completeDefinition": true, "tagUsed": "struct", "id": "0x2", "inner": [
            {"kind": "CXXRecordDecl", "name": "Foo", "tagUsed": "struct", "id": "0x3"}, 
            {"kind": "CXXConstructorDecl", "name": "Foo", "explicitlyDefaulted": "default", "inline": true, "id": "0x4", "type": {"qualType": "void ()"}}, 
            {"kind": "CXXConstructorDecl", "name": "Foo", "explicitlyDefaulted": "default", "inline": true, "id": "0x5", "type": {"qualType": "void (const Foo &)"}, "inner": [
            {"kind": "ParmVarDecl", "id": "0x6", "type": {"qualType": "const Foo &"}}]}, 
            {"kind": "CXXConstructorDecl", "name": "Foo", "explicitlyDefaulted": "default", "inline": true, "id": "0x7", "type": {"qualType": "void (Foo &&) noexcept"}, "inner": [
            {"kind": "ParmVarDecl", "id": "0x8", "type": {"qualType": "Foo &&"}}, 
            {"kind": "CompoundStmt", "id": "0x9"}]}]}, 
            {"kind": "FunctionDecl", "name": "foo", "id": "0xa", "type": {"qualType": "Foo (Foo)"}, "inner": [
            {"kind": "ParmVarDecl", "name": "foo", "id": "0xb", "type": {"qualType": "Foo"}}, 
            {"kind": "CompoundStmt", "id": "0xc", "inner": [
            {"kind": "ReturnStmt", "id": "0xd", "inner": [
            {"kind": "CXXConstructExpr", "id": "0xe", "type": {"qualType": "Foo"}, "inner": [
            {"kind": "ImplicitCastExpr", "id": "0xf", "type": {"qualType": "Foo"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0x10", "type": {"qualType": "Foo"}, "referencedDecl": {"name": "foo", "id": "0xb"}}]}]}]}]}]}]})C";

        ASSERT_EQUALS("struct Foo { Foo ( ) = default ; Foo ( const Foo & ) = default ; Foo ( Foo && ) { } } ; Foo foo ( Foo foo@1 ) { return foo@1 ; }", parse(clang_json));
    }

    void cxxConstructExpr2() {
        // This json dump was generated by tools/testclangimport.py
        // code: namespace a { struct Foo {}; } a::Foo foo(a::Foo foo) { return foo; }
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "NamespaceDecl", "name": "a", "id": "0x2", "inner": [
            {"kind": "CXXRecordDecl", "name": "Foo", "completeDefinition": true, "tagUsed": "struct", "id": "0x3", "inner": [
            {"kind": "CXXRecordDecl", "name": "Foo", "tagUsed": "struct", "id": "0x4"}, 
            {"kind": "CXXConstructorDecl", "name": "Foo", "explicitlyDefaulted": "default", "inline": true, "id": "0x5", "type": {"qualType": "void ()"}}, 
            {"kind": "CXXConstructorDecl", "name": "Foo", "explicitlyDefaulted": "default", "inline": true, "id": "0x6", "type": {"qualType": "void (const Foo &)"}, "inner": [
            {"kind": "ParmVarDecl", "id": "0x7", "type": {"qualType": "const Foo &"}}]}, 
            {"kind": "CXXConstructorDecl", "name": "Foo", "explicitlyDefaulted": "default", "inline": true, "id": "0x8", "type": {"qualType": "void (Foo &&) noexcept"}, "inner": [
            {"kind": "ParmVarDecl", "id": "0x9", "type": {"qualType": "Foo &&"}}, 
            {"kind": "CompoundStmt", "id": "0xa"}]}]}]}, 
            {"kind": "FunctionDecl", "name": "foo", "id": "0xb", "type": {"qualType": "a::Foo (a::Foo)"}, "inner": [
            {"kind": "ParmVarDecl", "name": "foo", "id": "0xc", "type": {"qualType": "a::Foo"}}, 
            {"kind": "CompoundStmt", "id": "0xd", "inner": [
            {"kind": "ReturnStmt", "id": "0xe", "inner": [
            {"kind": "CXXConstructExpr", "id": "0xf", "type": {"qualType": "a::Foo"}, "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x10", "type": {"qualType": "a::Foo"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0x11", "type": {"qualType": "a::Foo"}, "referencedDecl": {"name": "foo", "id": "0xc"}}]}]}]}]}]}]})C";

        ASSERT_EQUALS("namespace a { struct Foo { Foo ( ) = default ; Foo ( const Foo & ) = default ; Foo ( Foo && ) { } } ; } a :: Foo foo ( a :: Foo foo@1 ) { return foo@1 ; }", parse(clang_json));
    }

    void cxxConstructExpr3() {
        // This json dump was generated by tools/testclangimport.py
        // code: namespace a { struct Foo {   Foo(const char* ptr); }; } void foo(const char* ptr) { a::Foo x(ptr); }
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "NamespaceDecl", "name": "a", "id": "0x2", "inner": [
            {"kind": "CXXRecordDecl", "name": "Foo", "completeDefinition": true, "tagUsed": "struct", "id": "0x3", "inner": [
            {"kind": "CXXRecordDecl", "name": "Foo", "tagUsed": "struct", "id": "0x4"}, 
            {"kind": "CXXConstructorDecl", "name": "Foo", "id": "0x5", "type": {"qualType": "void (const char *)"}, "inner": [
            {"kind": "ParmVarDecl", "name": "ptr", "id": "0x6", "type": {"qualType": "const char *"}}]}, 
            {"kind": "CXXConstructorDecl", "name": "Foo", "explicitlyDefaulted": "default", "inline": true, "id": "0x7", "type": {"qualType": "void (const Foo &)"}, "inner": [
            {"kind": "ParmVarDecl", "id": "0x8", "type": {"qualType": "const Foo &"}}]}, 
            {"kind": "CXXConstructorDecl", "name": "Foo", "explicitlyDefaulted": "default", "inline": true, "id": "0x9", "type": {"qualType": "void (Foo &&)"}, "inner": [
            {"kind": "ParmVarDecl", "id": "0xa", "type": {"qualType": "Foo &&"}}]}]}]}, 
            {"kind": "FunctionDecl", "name": "foo", "id": "0xb", "type": {"qualType": "void (const char *)"}, "inner": [
            {"kind": "ParmVarDecl", "name": "ptr", "id": "0xc", "type": {"qualType": "const char *"}}, 
            {"kind": "CompoundStmt", "id": "0xd", "inner": [
            {"kind": "DeclStmt", "id": "0xe", "inner": [
            {"kind": "VarDecl", "name": "x", "init": "call", "id": "0xf", "type": {"qualType": "a::Foo"}, "inner": [
            {"kind": "CXXConstructExpr", "id": "0x10", "type": {"qualType": "a::Foo"}, "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x11", "type": {"qualType": "const char *"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0x12", "type": {"qualType": "const char *"}, "referencedDecl": {"name": "ptr", "id": "0xc"}}]}]}]}]}]}]}]})C";

        ASSERT_EQUALS("namespace a { struct Foo { Foo ( const char * ptr@1 ) ; Foo ( const Foo & ) = default ; Foo ( Foo && ) = default ; } ; } void foo ( const char * ptr@2 ) { a :: Foo x@3 ; }", parse(clang_json));
    }
/*
    void cxxDeleteExpr() {
        const char clang[] = "|-FunctionDecl 0x2e0e740 <1.cpp:1:1, col:28> col:6 f 'void (int *)'\n"
                             "| |-ParmVarDecl 0x2e0e680 <col:8, col:13> col:13 used p 'int *'\n"
                             "| `-CompoundStmt 0x2e0ee70 <col:16, col:28>\n"
                             "|   `-CXXDeleteExpr 0x2e0ee48 <col:18, col:25> 'void' Function 0x2e0ebb8 'operator delete' 'void (void *) noexcept'\n"
                             "|     `-ImplicitCastExpr 0x2e0e850 <col:25> 'int *' <LValueToRValue>\n"
                             "|       `-DeclRefExpr 0x2e0e828 <col:25> 'int *' lvalue ParmVar 0x2e0e680 'p' 'int *'";
        ASSERT_EQUALS("void f ( int * p@1 ) { delete p@1 ; }", parse(clang));
    }

    void cxxDestructorDecl() {
        const char clang[] = "`-CXXRecordDecl 0x8ecd60 <1.cpp:1:1, line:4:1> line:1:8 struct S definition\n"
                             "  `-CXXDestructorDecl 0x8ed088 <line:3:3, col:9> col:3 ~S 'void () noexcept'\n"
                             "    `-CompoundStmt 0x8ed1a8 <col:8, col:9>";
        ASSERT_EQUALS("struct S { ~S ( ) { } } ;", parse(clang));
    }
 */
    void cxxForRangeStmt1() {
        // This json dump was generated by tools/testclangimport.py
        // code: int x[2]; void f () {   for (int i: x) {} }
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "VarDecl", "name": "x", "id": "0x2", "type": {"qualType": "int[2]"}}, 
            {"kind": "FunctionDecl", "name": "f", "id": "0x3", "type": {"qualType": "void ()"}, "inner": [
            {"kind": "CompoundStmt", "id": "0x4", "inner": [
            {"kind": "CXXForRangeStmt", "id": "0x5", "inner": [{}, 
            {"kind": "DeclStmt", "id": "0x6", "inner": [
            {"kind": "VarDecl", "name": "__range1", "init": "c", "id": "0x7", "type": {"qualType": "int (&)[2]"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0x8", "type": {"qualType": "int[2]"}, "referencedDecl": {"name": "x", "id": "0x2"}}]}]}, 
            {"kind": "DeclStmt", "id": "0x9", "inner": [
            {"kind": "VarDecl", "name": "__begin1", "init": "c", "id": "0xa", "type": {"qualType": "int *"}, "inner": [
            {"kind": "ImplicitCastExpr", "id": "0xb", "type": {"qualType": "int *"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0xc", "type": {"qualType": "int[2]"}, "referencedDecl": {"name": "__range1", "id": "0x7"}}]}]}]}, 
            {"kind": "DeclStmt", "id": "0xd", "inner": [
            {"kind": "VarDecl", "name": "__end1", "init": "c", "id": "0xe", "type": {"qualType": "int *"}, "inner": [
            {"kind": "BinaryOperator", "opcode": "+", "id": "0xf", "type": {"qualType": "int *"}, "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x10", "type": {"qualType": "int *"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0x11", "type": {"qualType": "int[2]"}, "referencedDecl": {"name": "__range1", "id": "0x7"}}]}, 
            {"kind": "IntegerLiteral", "value": "2", "id": "0x12", "type": {"qualType": "long"}}]}]}]}, 
            {"kind": "BinaryOperator", "opcode": "!=", "id": "0x13", "type": {"qualType": "bool"}, "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x14", "type": {"qualType": "int *"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0x15", "type": {"qualType": "int *"}, "referencedDecl": {"name": "__begin1", "id": "0xa"}}]}, 
            {"kind": "ImplicitCastExpr", "id": "0x16", "type": {"qualType": "int *"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0x17", "type": {"qualType": "int *"}, "referencedDecl": {"name": "__end1", "id": "0xe"}}]}]}, 
            {"kind": "UnaryOperator", "opcode": "++", "id": "0x18", "type": {"qualType": "int *"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0x19", "type": {"qualType": "int *"}, "referencedDecl": {"name": "__begin1", "id": "0xa"}}]}, 
            {"kind": "DeclStmt", "id": "0x1a", "inner": [
            {"kind": "VarDecl", "name": "i", "init": "c", "id": "0x1b", "type": {"qualType": "int"}, "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x1c", "type": {"qualType": "int"}, "inner": [
            {"kind": "UnaryOperator", "opcode": "*", "id": "0x1d", "type": {"qualType": "int"}, "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x1e", "type": {"qualType": "int *"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0x1f", "type": {"qualType": "int *"}, "referencedDecl": {"name": "__begin1", "id": "0xa"}}]}]}]}]}]}, 
            {"kind": "CompoundStmt", "id": "0x20"}]}]}]}]})C";

        ASSERT_EQUALS("int x@1 [ 2 ] ; void f ( ) { for ( int i@2 : x@1 ) { } }",
                      parse(clang_json));
    }
/*
    void cxxFunctionalCastExpr() {
        const char clang[] = "`-FunctionDecl 0x156fe98 <a.cpp:1:1, line:3:1> line:1:5 main 'int (int, char **)'\n"
                             "  |-ParmVarDecl 0x156fd00 <col:10, col:14> col:14 argc 'int'\n"
                             "  |-ParmVarDecl 0x156fdb8 <col:20, col:27> col:27 argv 'char **'\n"
                             "  `-CompoundStmt 0x1596410 <line:2:1, line:2:1>\n"
                             "    |-DeclStmt 0x15946a8 <line:2:15, line:2:29>\n"
                             "    | `-VarDecl 0x1570118 <line:2:15, line:2:28> col:11 used setCode 'MyVar<int>':'MyVar<int>' cinit\n"
                             "    |   `-ExprWithCleanups 0x1594690 <line:2:15, line:3:28> 'MyVar<int>':'MyVar<int>'\n"
                             "    |     `-CXXConstructExpr 0x1594660 <line:2:15, line:3:28> 'MyVar<int>':'MyVar<int>' 'void (MyVar<int> &&) noexcept' elidable\n"
                             "    |       `-MaterializeTemporaryExpr 0x1592b68 <line:2:15, line:3:28> 'MyVar<int>':'MyVar<int>' xvalue\n"
                             "    |         `-CXXFunctionalCastExpr 0x1592b40 <line:2:15, line:3:28> 'MyVar<int>':'MyVar<int>' functional cast to MyVar<int> <ConstructorConversion>\n"
                             "    |           `-CXXConstructExpr 0x15929f0 <line:2:15, line:3:28> 'MyVar<int>':'MyVar<int>' 'void (int)'\n"
                             "    |             `-IntegerLiteral 0x1570248 <col:27> 'int' 5\n";
        ASSERT_EQUALS("int main ( int argc@1 , char * * argv@2 ) { MyVar<int> setCode@3 = MyVar<int> ( 5 ) ; }",
                      parse(clang));
    }

    void cxxMemberCall() {
        const char clang[] = "`-FunctionDecl 0x320dc80 <a.cpp:2:1, col:33> col:6 bar 'void ()'\n"
                             "  `-CompoundStmt 0x323bb08 <col:12, col:33>\n"
                             "    |-DeclStmt 0x323ba40 <col:14, col:22>\n"
                             "    | `-VarDecl 0x320df28 <col:14, col:21> col:21 used c 'C<int>':'C<int>' callinit\n"
                             "    |   `-CXXConstructExpr 0x323ba10 <col:21> 'C<int>':'C<int>' 'void () noexcept'\n"
                             "    `-CXXMemberCallExpr 0x323bab8 <col:24, col:30> 'int':'int'\n"
                             "      `-MemberExpr 0x323ba80 <col:24, col:26> '<bound member function type>' .foo 0x320e160\n"
                             "        `-DeclRefExpr 0x323ba58 <col:24> 'C<int>':'C<int>' lvalue Var 0x320df28 'c' 'C<int>':'C<int>'";
        ASSERT_EQUALS("void bar ( ) { C<int> c@1 ( C<int> ( ) ) ; c@1 . foo ( ) ; }", parse(clang));
    }

    void cxxMethodDecl1() {
        const char clang[] = "|-CXXMethodDecl 0x55c786f5ad60 <a.cpp:56:5, col:179> col:10 analyzeFile '_Bool (const std::string &, const std::string &, const std::string &, unsigned long long, std::list<ErrorLogger::ErrorMessage> *)'\n"
                             "| |-ParmVarDecl 0x55c786f5a4c8 <col:22, col:41> col:41 buildDir 'const std::string &'\n"
                             "| |-ParmVarDecl 0x55c786f5a580 <col:51, col:70> col:70 sourcefile 'const std::string &'\n"
                             "| |-ParmVarDecl 0x55c786f5a638 <col:82, col:101> col:101 cfg 'const std::string &'\n"
                             "| |-ParmVarDecl 0x55c786f5a6a8 <col:106, col:125> col:125 checksum 'unsigned long long'\n"
                             "| |-ParmVarDecl 0x55c786f5ac00 <col:135, col:173> col:173 errors 'std::list<ErrorLogger::ErrorMessage> *'\n"
                             "  `-CompoundStmt 0x0 <>";
        ASSERT_EQUALS("_Bool analyzeFile ( const std :: string & buildDir@1 , const std :: string & sourcefile@2 , const std :: string & cfg@3 , unsigned long long checksum@4 , std::list<ErrorLogger::ErrorMessage> * errors@5 ) { }", parse(clang));
    }

    void cxxMethodDecl2() { // "unexpanded" template method
        const char clang[] = "`-CXXMethodDecl 0x220ecb0 parent 0x21e4c28 prev 0x21e5338 <a.cpp:11:1, line:18:1> line:14:1 find 'const typename char_traits<_CharT>::char_type *(const char_traits::char_type *, int, const char_traits::char_type &)'\n"
                             "  `-CompoundStmt 0x220ede0 <line:15:1, line:18:1>\n"
                             "    `-ReturnStmt 0x220edd0 <line:17:5, col:12>\n"
                             "      `-IntegerLiteral 0x220edb0 <col:12> 'int' 0";
        ASSERT_EQUALS("", parse(clang));
    }

    void cxxMethodDecl3() {
        const char clang[] = "|-CXXRecordDecl 0x21cca40 <2.cpp:2:1, line:4:1> line:2:7 class Fred definition\n"
                             "| |-DefinitionData pass_in_registers empty aggregate standard_layout trivially_copyable pod trivial literal has_constexpr_non_copy_move_ctor can_const_default_init\n"
                             "| | |-DefaultConstructor exists trivial constexpr needs_implicit defaulted_is_constexpr\n"
                             "| | |-CopyConstructor simple trivial has_const_param needs_implicit implicit_has_const_param\n"
                             "| | |-MoveConstructor exists simple trivial needs_implicit\n"
                             "| | |-CopyAssignment trivial has_const_param needs_implicit implicit_has_const_param\n"
                             "| | |-MoveAssignment exists simple trivial needs_implicit\n"
                             "| | `-Destructor simple irrelevant trivial needs_implicit\n"
                             "| |-CXXRecordDecl 0x21ccb58 <col:1, col:7> col:7 implicit class Fred\n"
                             "| `-CXXMethodDecl 0x21ccc68 <line:3:1, col:10> col:6 foo 'void ()'\n"
                             "`-CXXMethodDecl 0x21ccd60 parent 0x21cca40 prev 0x21ccc68 <line:6:1, col:19> col:12 foo 'void ()'\n"
                             "  `-CompoundStmt 0x21cce50 <col:18, col:19>";
        ASSERT_EQUALS("class Fred { void foo ( ) ; } ; void Fred :: foo ( ) { }", parse(clang));
    }

    void cxxMethodDecl4() {
        const char clang[] = "|-ClassTemplateSpecializationDecl 0x15d82f8 <a.cpp:7:3, line:18:3> line:8:12 struct char_traits definition\n"
                             "| |-TemplateArgument type 'char'\n"
                             "| | `-BuiltinType 0x15984c0 'char'\n"
                             "| |-CXXRecordDecl 0x15d8520 <col:5, col:12> col:12 implicit struct char_traits\n"
                             "| |-CXXMethodDecl 0x15d8738 <line:12:7, line:16:7> line:13:7 move 'char *(char *)' static\n"
                             "| | |-ParmVarDecl 0x15d8630 <col:12, col:18> col:18 used __s1 'char *'\n"
                             "| | `-CompoundStmt 0x15d88e8 <line:14:7, line:16:7>\n";
        ASSERT_EQUALS("struct char_traits<char> { static char * move ( char * __s1@1 ) { } } ;",
                      parse(clang));
    }

    void cxxNewExpr1() {
        const char clang[] = "|-VarDecl 0x3a97680 <1.cpp:2:1, col:14> col:6 i 'int *' cinit\n"
                             "| `-CXXNewExpr 0x3a97d18 <col:10, col:14> 'int *' Function 0x3a97778 'operator new' 'void *(unsigned long)'\n"
                             "`-VarDecl 0x3a97d80 <line:3:1, col:21> col:6 j 'int *' cinit\n"
                             "  `-CXXNewExpr 0x3a97e68 <col:10, col:21> 'int *' array Function 0x3a978c0 'operator new[]' 'void *(unsigned long)'\n"
                             "    `-ImplicitCastExpr 0x3a97e18 <col:18> 'unsigned long' <IntegralCast>\n"
                             "      `-IntegerLiteral 0x3a97de0 <col:18> 'int' 100";
        ASSERT_EQUALS("int * i@1 = new int ; int * j@2 = new int [ 100 ] ;",
                      parse(clang));
    }

    void cxxNewExpr2() {
        const char clang[] = "|-FunctionDecl 0x59a188 <a.cpp:7:1, line:9:1> line:7:11 f 'struct S *()'\n"
                             "| `-CompoundStmt 0x5c4318 <col:15, line:9:1>\n"
                             "|   `-ReturnStmt 0x5c4308 <line:8:3, col:14>\n"
                             "|     `-CXXNewExpr 0x5c42c8 <col:10, col:14> 'S *' Function 0x59a378 'operator new' 'void *(unsigned long)'\n"
                             "|       `-CXXConstructExpr 0x5c42a0 <col:14> 'S' 'void () noexcept'";
        ASSERT_EQUALS("struct S * f ( ) { return new S ( ) ; }",
                      parse(clang));
    }

    void cxxNullPtrLiteralExpr() {
        const char clang[] = "`-VarDecl 0x2a7d650 <1.cpp:1:1, col:17> col:13 p 'const char *' cinit\n"
                             "  `-ImplicitCastExpr 0x2a7d708 <col:17> 'const char *' <NullToPointer>\n"
                             "    `-CXXNullPtrLiteralExpr 0x2a7d6f0 <col:17> 'nullptr_t'";
        ASSERT_EQUALS("const char * p@1 = nullptr ;", parse(clang));
    }

    void cxxOperatorCallExpr() {
        const char clang[] = "`-FunctionDecl 0x3c099f0 <a.cpp:2:1, col:24> col:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x3c37308 <col:12, col:24>\n"
                             "    |-DeclStmt 0x3c0a060 <col:14, col:17>\n"
                             "    | `-VarDecl 0x3c09ae0 <col:14, col:16> col:16 used c 'C' callinit\n"
                             "    |   `-CXXConstructExpr 0x3c0a030 <col:16> 'C' 'void () noexcept'\n"
                             "    `-CXXOperatorCallExpr 0x3c372c0 <col:19, col:21> 'void'\n"
                             "      |-ImplicitCastExpr 0x3c372a8 <col:20> 'void (*)(int)' <FunctionToPointerDecay>\n"
                             "      | `-DeclRefExpr 0x3c37250 <col:20> 'void (int)' lvalue CXXMethod 0x3c098c0 'operator=' 'void (int)'\n"
                             "      |-DeclRefExpr 0x3c0a078 <col:19> 'C' lvalue Var 0x3c09ae0 'c' 'C'\n"
                             "      `-IntegerLiteral 0x3c0a0a0 <col:21> 'int' 4";
        ASSERT_EQUALS("void foo ( ) { C c@1 ( C ( ) ) ; c@1 . operator= ( 4 ) ; }", parse(clang));
    }

    void cxxRecordDecl1() {
        const char* clang = "`-CXXRecordDecl 0x34cc5f8 <1.cpp:2:1, col:7> col:7 class Foo";
        ASSERT_EQUALS("class Foo ;", parse(clang));

        clang = "`-CXXRecordDecl 0x34cc5f8 <C:\\Foo\\Bar Baz\\1.cpp:2:1, col:7> col:7 class Foo";
        ASSERT_EQUALS("class Foo ;", parse(clang));

        clang = "`-CXXRecordDecl 0x34cc5f8 <C:/Foo/Bar Baz/1.cpp:2:1, col:7> col:7 class Foo";
        ASSERT_EQUALS("class Foo ;", parse(clang));
    }

    void cxxRecordDecl2() {
        const char clang[] = "`-CXXRecordDecl 0x34cc5f8 <1.cpp:2:1, col:7> col:7 struct Foo definition";
        ASSERT_EQUALS("struct Foo { } ;", parse(clang));
    }

    void cxxRecordDeclDerived() {
        const char clang[] = "|-CXXRecordDecl 0x19ccd38 <e.cpp:4:1, line:6:1> line:4:8 referenced struct base definition\n"
                             "| `-VarDecl 0x19ccf00 <line:5:5, col:35> col:27 value 'const bool' static constexpr cinit\n"
                             "|   |-value: Int 0\n"
                             "|   `-CXXBoolLiteralExpr 0x19ccf68 <col:35> 'bool' false\n"
                             "`-CXXRecordDecl 0x19ccfe8 <line:8:1, col:32> col:8 struct derived definition\n"
                             "  |-public 'base'\n"
                             "  `-CXXRecordDecl 0x19cd150 <col:1, col:8> col:8 implicit struct derived";

        ASSERT_EQUALS("struct base { static const bool value@1 = false ; } ; struct derived : public base { } ;", parse(clang));
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

    void cxxStaticCastExpr3() {
        const char clang[] = "`-ClassTemplateSpecializationDecl 0xd842d8 <a.cpp:4:3, line:7:3> line:4:21 struct char_traits definition\n"
                             "  |-TemplateArgument type 'char'\n"
                             "  | `-BuiltinType 0xd444c0 'char'\n"
                             "  |-CXXRecordDecl 0xd84500 <col:14, col:21> col:21 implicit struct char_traits\n"
                             "  |-TypedefDecl 0xd845a0 <line:5:7, col:20> col:20 referenced char_type 'char'\n"
                             "  | `-BuiltinType 0xd444c0 'char'\n"
                             "  `-CXXMethodDecl 0xd847b0 <line:6:7, col:80> col:18 assign 'char_traits<char>::char_type *(char_traits<char>::char_type *)'\n"
                             "    |-ParmVarDecl 0xd84670 <col:25, col:36> col:36 used __s 'char_traits<char>::char_type *'\n"
                             "    `-CompoundStmt 0xd848f8 <col:41, col:80>\n"
                             "      `-ReturnStmt 0xd848e8 <col:43, col:77>\n"
                             "        `-CXXStaticCastExpr 0xd848b8 <col:50, col:77> 'char_traits<char>::char_type *' static_cast<char_traits<char>::char_type *> <NoOp>\n"
                             "          `-ImplicitCastExpr 0xd848a0 <col:74> 'char_traits<char>::char_type *' <LValueToRValue> part_of_explicit_cast\n"
                             "            `-DeclRefExpr 0xd84870 <col:74> 'char_traits<char>::char_type *' lvalue ParmVar 0xd84670 '__s' 'char_traits<char>::char_type *'\n";

        ASSERT_EQUALS("struct char_traits<char> { typedef char char_type ; char_traits<char>::char_type * assign ( char_traits<char>::char_type * __s@1 ) { return static_cast<char_traits<char>::char_type*> ( __s@1 ) ; } } ;", parse(clang));
    }

    void cxxStdInitializerListExpr() {
        const char clang[] = "`-VarDecl 0x2f92060 <1.cpp:3:1, col:25> col:18 x 'std::vector<int>':'std::vector<int, std::allocator<int> >' listinit\n"
                             "  `-ExprWithCleanups 0x2fb0b40 <col:18, col:25> 'std::vector<int>':'std::vector<int, std::allocator<int> >'\n"
                             "    `-CXXConstructExpr 0x2fb0b00 <col:18, col:25> 'std::vector<int>':'std::vector<int, std::allocator<int> >' 'void (initializer_list<std::vector<int, std::allocator<int> >::value_type>, const std::vector<int, std::allocator<int> >::allocator_type &)' list std::initializer_list\n"
                             "      |-CXXStdInitializerListExpr 0x2fb0928 <col:19, col:25> 'initializer_list<std::vector<int, std::allocator<int> >::value_type>':'std::initializer_list<int>'\n"
                             "      | `-MaterializeTemporaryExpr 0x2fb0910 <col:19, col:25> 'const int [3]' xvalue\n"
                             "      |   `-InitListExpr 0x2fb08b8 <col:19, col:25> 'const int [3]'\n"
                             "      |     |-IntegerLiteral 0x2f920c0 <col:20> 'int' 1\n"
                             "      |     |-IntegerLiteral 0x2f920e0 <col:22> 'int' 2\n"
                             "      |     `-IntegerLiteral 0x2f92100 <col:24> 'int' 3\n"
                             "      `-CXXDefaultArgExpr 0x2fb0ae0 <<invalid sloc>> 'const std::vector<int, std::allocator<int> >::allocator_type':'const std::allocator<int>' lvalue";
        ASSERT_EQUALS("std :: vector<int> x@1 { 1 , 2 , 3 } ;", parse(clang));
    }

    void cxxThrowExpr() {
        const char clang[] = "`-FunctionDecl 0x3701690 <1.cpp:2:1, col:23> col:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x37017b0 <col:12, col:23>\n"
                             "    `-CXXThrowExpr 0x3701790 <col:14, col:20> 'void'\n"
                             "      `-IntegerLiteral 0x3701770 <col:20> 'int' 1";
        ASSERT_EQUALS("void foo ( ) { throw 1 ; }", parse(clang));
    }
 */
    void defaultStmt() {
        // This json dump was generated by tools/testclangimport.py
        // code: int foo ( int rc ) { switch ( rc ) { default : return 1 ; } }
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "foo", "type": {"qualType": "int (int)"}, "inner": [
            {"kind": "ParmVarDecl", "id": "0x3", "name": "rc", "type": {"qualType": "int"}}, 
            {"kind": "CompoundStmt", "id": "0x4", "inner": [
            {"kind": "SwitchStmt", "id": "0x5", "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x6", "type": {"qualType": "int"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0x7", "type": {"qualType": "int"}, "referencedDecl": {"name": "rc", "id": "0x3"}}]}, 
            {"kind": "CompoundStmt", "id": "0x8", "inner": [
            {"kind": "DefaultStmt", "id": "0x9", "inner": [
            {"kind": "ReturnStmt", "id": "0xa", "inner": [
            {"kind": "IntegerLiteral", "id": "0xb", "value": "1", "type": {"qualType": "int"}}]}]}]}]}]}]}]})C";

        ASSERT_EQUALS("int foo ( int rc@1 ) { switch ( rc@1 ) { default : return 1 ; } }", parse(clang_json));
    }

    void doStmt() {
        // This json dump was generated by tools/testclangimport.py
        // code: void foo ( ) { do { } while ( 1 ) ; }
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "foo", "type": {"qualType": "void ()"}, "inner": [
            {"kind": "CompoundStmt", "id": "0x3", "inner": [
            {"kind": "DoStmt", "id": "0x4", "inner": [
            {"kind": "CompoundStmt", "id": "0x5"}, 
            {"kind": "IntegerLiteral", "id": "0x6", "value": "1", "type": {"qualType": "int"}}]}]}]}]})C";
        ASSERT_EQUALS("void foo ( ) { do { } while ( 1 ) ; }", parse(clang_json));
    }

    void enumDecl1() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "EnumDecl", "id": "0x2", "name": "abc", "inner": [
            {"kind": "EnumConstantDecl", "id": "0x3", "name": "a", "type": {"qualType": "int"}}, 
            {"kind": "EnumConstantDecl", "id": "0x4", "name": "b", "type": {"qualType": "int"}}, 
            {"kind": "EnumConstantDecl", "id": "0x5", "name": "c", "type": {"qualType": "int"}}]}]})C";
        ASSERT_EQUALS("enum abc { a , b , c }", parse(clang_json));
    }

    void enumDecl2() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "EnumDecl", "id": "0x2", "name": "x", "fixedUnderlyingType": {"qualType": "unsigned int"}}]})C";
        ASSERT_EQUALS("enum x : unsigned int { }", parse(clang_json));
    }

    void enumDecl4() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "EnumDecl", "id": "0x2", "inner": [
            {"kind": "EnumConstantDecl", "id": "0x3", "name": "A", "type": {"qualType": "int"}}, 
            {"kind": "EnumConstantDecl", "id": "0x4", "name": "B", "type": {"qualType": "int"}}, 
            {"kind": "EnumConstantDecl", "id": "0x5", "name": "C", "type": {"qualType": "int"}}]}, 
            {"kind": "VarDecl", "id": "0x6", "name": "x", "type": {"qualType": "enum (unnamed enum at 1.c:2:1)"}, "init": "c", "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x7", "type": {"qualType": "enum (unnamed enum at 1.c:2:1)"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0x8", "type": {"qualType": "int"}, "referencedDecl": {"name": "C", "id": "0x5"}}]}]}]})C";
        ASSERT_EQUALS("enum { A , B , C } x@1 = C ;", parse(clang_json));
    }

    void floatLiteral() {
        // This json dump was generated by tools/testclangimport.py
        // code: float f = 0.1f;
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "VarDecl", "id": "0x2", "name": "f", "init": "c", "type": {"qualType": "float"}, "inner": [
            {"kind": "FloatingLiteral", "id": "0x3", "value": "0.100000001", "type": {"qualType": "float"}}]}]})C";
        ASSERT_EQUALS("float f@1 = 0.100000001 ;", parse(clang_json));
    }

    void forStmt() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "foo", "type": {"qualType": "void ()"}, "inner": [
            {"kind": "CompoundStmt", "id": "0x3", "inner": [
            {"kind": "ForStmt", "id": "0x4", "inner": [
            {"kind": "DeclStmt", "id": "0x5", "inner": [
            {"kind": "VarDecl", "id": "0x6", "name": "i", "type": {"qualType": "int"}, "init": "c", "inner": [
            {"kind": "IntegerLiteral", "id": "0x7", "value": "0", "type": {"qualType": "int"}}]}]}, {}, 
            {"kind": "BinaryOperator", "id": "0x8", "opcode": "<", "type": {"qualType": "int"}, "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x9", "type": {"qualType": "int"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0xa", "type": {"qualType": "int"}, "referencedDecl": {"name": "i", "id": "0x6"}}]}, 
            {"kind": "IntegerLiteral", "id": "0xb", "value": "10", "type": {"qualType": "int"}}]}, 
            {"kind": "UnaryOperator", "id": "0xc", "opcode": "++", "type": {"qualType": "int"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0xd", "type": {"qualType": "int"}, "referencedDecl": {"name": "i", "id": "0x6"}}]}, 
            {"kind": "CompoundStmt", "id": "0xe"}]}]}]}]})C";
        ASSERT_EQUALS("void foo ( ) { for ( int i@1 = 0 ; i@1 < 10 ; ++ i@1 ) { } }", parse(clang_json));
    }

    void funcdecl1() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "foo", "type": {"qualType": "void (int, int)"}, "inner": [
            {"kind": "ParmVarDecl", "id": "0x3", "name": "x", "type": {"qualType": "int"}}, 
            {"kind": "ParmVarDecl", "id": "0x4", "name": "y", "type": {"qualType": "int"}}]}]})C";
        ASSERT_EQUALS("void foo ( int x@1 , int y@2 ) ;", parse(clang_json));
    }

    void funcdecl2() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "foo", "type": {"qualType": "int (int, int)"}, "inner": [
            {"kind": "ParmVarDecl", "id": "0x3", "name": "x", "type": {"qualType": "int"}}, 
            {"kind": "ParmVarDecl", "id": "0x4", "name": "y", "type": {"qualType": "int"}}, 
            {"kind": "CompoundStmt", "id": "0x5", "inner": [
            {"kind": "ReturnStmt", "id": "0x6", "inner": [
            {"kind": "BinaryOperator", "id": "0x7", "opcode": "/", "type": {"qualType": "int"}, "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x8", "type": {"qualType": "int"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0x9", "type": {"qualType": "int"}, "referencedDecl": {"name": "x", "id": "0x3"}}]}, 
            {"kind": "ImplicitCastExpr", "id": "0xa", "type": {"qualType": "int"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0xb", "type": {"qualType": "int"}, "referencedDecl": {"name": "y", "id": "0x4"}}]}]}]}]}]}]})C";
        ASSERT_EQUALS("int foo ( int x@1 , int y@2 ) { return x@1 / y@2 ; }", parse(clang_json));
    }

    void funcdecl3() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "foo", "type": {"qualType": "void (int *, int)"}, "inner": [
            {"kind": "ParmVarDecl", "id": "0x3", "type": {"qualType": "int *"}}, 
            {"kind": "ParmVarDecl", "id": "0x4", "type": {"qualType": "int"}}]}]})C";
        ASSERT_EQUALS("void foo ( int * , int ) ;", parse(clang_json));
    }

    void funcdecl5() {
        // This json dump was generated by tools/testclangimport.py
        // code: static inline void foo ( ) ;
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "name": "foo", "inline": true, "storageClass": "static", "id": "0x2", "type": {"qualType": "void ()"}}]})C";
        ASSERT_EQUALS("static inline void foo ( ) ;", parse(clang_json));
    }

/*
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
        ASSERT_EQUALS("int foo<int> ( int t@1 ) { return t@1 + 1 ; } int bar ( ) { foo ( 1 ) ; }", parse(clang));
    }
 */
    void ifelse() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "foo", "type": {"qualType": "int (int)"}, "inner": [
            {"kind": "ParmVarDecl", "id": "0x3", "name": "x", "type": {"qualType": "int"}}, 
            {"kind": "CompoundStmt", "id": "0x4", "inner": [
            {"kind": "IfStmt", "id": "0x5", "inner": [
            {"kind": "BinaryOperator", "id": "0x6", "opcode": ">", "type": {"qualType": "int"}, "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x7", "type": {"qualType": "int"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0x8", "type": {"qualType": "int"}, "referencedDecl": {"name": "x", "id": "0x3"}}]}, 
            {"kind": "IntegerLiteral", "id": "0x9", "value": "10", "type": {"qualType": "int"}}]}, 
            {"kind": "CompoundStmt", "id": "0xa"}, 
            {"kind": "CompoundStmt", "id": "0xb"}]}]}]}]})C";
        ASSERT_EQUALS("int foo ( int x@1 ) { if ( x@1 > 10 ) { } else { } }", parse(clang_json));
    }

    void ifStmt() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "foo", "type": {"qualType": "void ()"}, "inner": [
            {"kind": "CompoundStmt", "id": "0x3", "inner": [
            {"kind": "IfStmt", "id": "0x4", "inner": [
            {"kind": "IntegerLiteral", "id": "0x5", "value": "1", "type": {"qualType": "int"}}, 
            {"kind": "CompoundStmt", "id": "0x6"}]}]}]}]})C";
        ASSERT_EQUALS("void foo ( ) { if ( 1 ) { } }", parse(clang_json));
    }
/*
    void initListExpr() {
        const char clang[] = "|-VarDecl 0x397c680 <1.cpp:2:1, col:26> col:11 used ints 'const int [3]' cinit\n"
                             "| `-InitListExpr 0x397c7d8 <col:20, col:26> 'const int [3]'\n"
                             "|   |-IntegerLiteral 0x397c720 <col:21> 'int' 1\n"
                             "|   |-IntegerLiteral 0x397c740 <col:23> 'int' 2\n"
                             "|   `-IntegerLiteral 0x397c760 <col:25> 'int' 3";
        ASSERT_EQUALS("const int [3] ints@1 = { 1 , 2 , 3 } ;", parse(clang));
    }
 */
    void labelStmt() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "foo", "type": {"qualType": "void ()"}, "inner": [
            {"kind": "CompoundStmt", "id": "0x3", "inner": [
            {"kind": "LabelStmt", "id": "0x4", "name": "loop", "declId": "0x5", "inner": [
            {"kind": "GotoStmt", "id": "0x6", "targetLabelDeclId": "0x5"}]}]}]}]})C";
        ASSERT_EQUALS("void foo ( ) { loop : goto loop ; }", parse(clang_json));
    }

    void memberExpr() {
        // This json dump was generated by tools/testclangimport.py
        // code: struct S { int x; }; int foo(struct S s) { return s.x; }
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "RecordDecl", "id": "0x2", "name": "S", "completeDefinition": true, "tagUsed": "struct", "inner": [
            {"kind": "FieldDecl", "id": "0x3", "name": "x", "type": {"qualType": "int"}}]}, 
            {"kind": "FunctionDecl", "id": "0x4", "name": "foo", "type": {"qualType": "int (struct S)"}, "inner": [
            {"kind": "ParmVarDecl", "id": "0x5", "name": "s", "type": {"qualType": "struct S"}}, 
            {"kind": "CompoundStmt", "id": "0x6", "inner": [
            {"kind": "ReturnStmt", "id": "0x7", "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x8", "type": {"qualType": "int"}, "inner": [
            {"kind": "MemberExpr", "id": "0x9", "name": "x", "referencedMemberDecl": "0x3", "type": {"qualType": "int"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0xa", "type": {"qualType": "struct S"}, "referencedDecl": {"name": "s", "id": "0x5"}}]}]}]}]}]}]})C";
        ASSERT_EQUALS("struct S { int x@1 ; } ; int foo ( struct S s@2 ) { return s@2 . x@1 ; }",
                      parse(clang_json));
    }

    void namespaceDecl1() {
        // This json dump was generated by tools/testclangimport.py
        // code: namespace x { int var ; }
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "NamespaceDecl", "name": "x", "id": "0x2", "inner": [
            {"kind": "VarDecl", "name": "var", "id": "0x3", "type": {"qualType": "int"}}]}]})C";

        ASSERT_EQUALS("namespace x { int var@1 ; }",
                      parse(clang_json));
    }

    void namespaceDecl2() {
        // This json dump was generated by tools/testclangimport.py
        // code: namespace { int var ; }
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "NamespaceDecl", "id": "0x2", "inner": [
            {"kind": "VarDecl", "name": "var", "id": "0x3", "type": {"qualType": "int"}}]}, 
            {"kind": "UsingDirectiveDecl", "id": "0x4"}]})C";

        ASSERT_EQUALS("namespace { int var@1 ; }",
                      parse(clang_json));
    }

    void recordDecl1() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "RecordDecl", "id": "0x2", "name": "S", "completeDefinition": true, "tagUsed": "struct", "inner": [
            {"kind": "FieldDecl", "id": "0x3", "name": "x", "type": {"qualType": "int"}}, 
            {"kind": "FieldDecl", "id": "0x4", "name": "y", "type": {"qualType": "int"}}]}]})C";
        ASSERT_EQUALS("struct S { int x@1 ; int y@2 ; } ;",
                      parse(clang_json));
    }

    void unaryExprOrTypeTraitExpr1() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "VarDecl", "id": "0x2", "name": "x", "init": "c", "type": {"qualType": "int"}, "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x3", "type": {"qualType": "int"}, "inner": [
            {"kind": "UnaryExprOrTypeTraitExpr", "id": "0x4", "name": "sizeof", "type": {"qualType": "unsigned long"}, "argType": {"qualType": "int"}}]}]}]})C";
        ASSERT_EQUALS("int x@1 = sizeof ( int ) ;", parse(clang_json));
    }

    void unaryExprOrTypeTraitExpr2() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "VarDecl", "id": "0x2", "name": "buf", "type": {"qualType": "short[10]"}}, 
            {"kind": "VarDecl", "id": "0x3", "name": "x", "init": "c", "type": {"qualType": "int"}, "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x4", "type": {"qualType": "int"}, "inner": [
            {"kind": "UnaryExprOrTypeTraitExpr", "id": "0x5", "name": "sizeof", "type": {"qualType": "unsigned long"}, "inner": [
            {"kind": "ParenExpr", "id": "0x6", "type": {"qualType": "short[10]"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0x7", "type": {"qualType": "short[10]"}, "referencedDecl": {"name": "buf", "id": "0x2"}}]}]}]}]}]})C";
        ASSERT_EQUALS("short buf@1 [ 10 ] ; int x@2 = sizeof ( buf@1 ) ;", parse(clang_json));
    }

    void unaryOperator() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "foo", "type": {"qualType": "int (int *)"}, "inner": [
            {"kind": "ParmVarDecl", "id": "0x3", "name": "p", "type": {"qualType": "int *"}}, 
            {"kind": "CompoundStmt", "id": "0x4", "inner": [
            {"kind": "ReturnStmt", "id": "0x5", "inner": [
            {"kind": "BinaryOperator", "id": "0x6", "opcode": "/", "type": {"qualType": "int"}, "inner": [
            {"kind": "IntegerLiteral", "id": "0x7", "value": "100000", "type": {"qualType": "int"}}, 
            {"kind": "ImplicitCastExpr", "id": "0x8", "type": {"qualType": "int"}, "inner": [
            {"kind": "UnaryOperator", "id": "0x9", "opcode": "*", "type": {"qualType": "int"}, "inner": [
            {"kind": "ImplicitCastExpr", "id": "0xa", "type": {"qualType": "int *"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0xb", "type": {"qualType": "int *"}, "referencedDecl": {"name": "p", "id": "0x3"}}]}]}]}]}]}]}]}]})C";
        ASSERT_EQUALS("int foo ( int * p@1 ) { return 100000 / * p@1 ; }", parse(clang_json));
    }

    void vardecl1() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "VarDecl", "id": "0x2", "name": "a", "init": "c", "type": {"qualType": "int"}, "inner": [
            {"kind": "IntegerLiteral", "id": "0x3", "value": "1", "type": {"qualType": "int"}}]}, 
            {"kind": "VarDecl", "id": "0x4", "name": "b", "type": {"qualType": "int"}}]})C";
        ASSERT_EQUALS("int a@1 = 1 ; int b@2 ;",
                      parse(clang_json));
    }

    void vardecl2() {
        // code: int a[10] ; void foo ( ) { a [ 0 ] = 0 ; }
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "VarDecl", "id": "0x2", "name": "a", "type": {"qualType": "int[10]"}}, 
            {"kind": "FunctionDecl", "id": "0x3", "name": "foo", "type": {"qualType": "void ()"}, "inner": [
            {"kind": "CompoundStmt", "id": "0x4", "inner": [
            {"kind": "BinaryOperator", "id": "0x5", "opcode": "=", "type": {"qualType": "int"}, "inner": [
            {"kind": "ArraySubscriptExpr", "id": "0x6", "type": {"qualType": "int"}, "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x7", "type": {"qualType": "int *"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0x8", "type": {"qualType": "int[10]"}, "referencedDecl": {"name": "a", "id": "0x2"}}]}, 
            {"kind": "IntegerLiteral", "id": "0x9", "value": "0", "type": {"qualType": "int"}}]}, 
            {"kind": "IntegerLiteral", "id": "0xa", "value": "0", "type": {"qualType": "int"}}]}]}]}]})C";

        ASSERT_EQUALS("int a@1 [ 10 ] ; void foo ( ) { a@1 [ 0 ] = 0 ; }",
                      parse(clang_json));
    }

    void vardecl3() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "VarDecl", "id": "0x2", "name": "p", "type": {"qualType": "const int *"}}]})C";
        ASSERT_EQUALS("const int * p@1 ;", parse(clang_json));
    }

    void vardecl4() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "TypedefDecl", "id": "0x2", "name": "PTR", "type": {"qualType": "void *"}, "inner": [
            {"kind": "PointerType", "id": "0x3", "type": {"qualType": "void *"}, "inner": [
            {"kind": "BuiltinType", "id": "0x4", "type": {"qualType": "void"}}]}]}, 
            {"kind": "VarDecl", "id": "0x5", "name": "p", "type": {"qualType": "PTR"}}]})C";

        ASSERT_EQUALS("typedef void * PTR ; PTR p@1 ;", parse(clang_json));
    }

    void vardecl6() {
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "VarDecl", "id": "0x2", "name": "x", "storageClass": "static", "type": {"qualType": "int"}}]})C";
        ASSERT_EQUALS("static int x@1 ;", parse(clang_json));
    }

    void vardecl7() {
        // code: void* (*fptr)(void);
        const std::string clang_json = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "VarDecl", "id": "0x2", "name": "fptr", "type": {"qualType": "void *(*)(void)"}}]})C";
        ASSERT_EQUALS("void * * fptr@1 ;", parse(clang_json));
    }

    void whileStmt1() {
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "foo", "type": {"qualType": "void ()"}, "inner": [
            {"kind": "CompoundStmt", "id": "0x3", "inner": [
            {"kind": "WhileStmt", "id": "0x4", "inner": [
            {"kind": "IntegerLiteral", "id": "0x5", "value": "0", "type": {"qualType": "int"}}, 
            {"kind": "CompoundStmt", "id": "0x6", "inner": [
            {"kind": "NullStmt", "id": "0x7"}]}]}]}]}]})C";
        ASSERT_EQUALS("void foo ( ) { while ( 0 ) { ; } }",
                      parse(clang_json));
    }

    void whileStmt2() {
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "foo", "type": {"qualType": "void ()"}, "inner": [
            {"kind": "CompoundStmt", "id": "0x3", "inner": [
            {"kind": "WhileStmt", "id": "0x4", "inner": [
            {"kind": "IntegerLiteral", "id": "0x5", "value": "1", "type": {"qualType": "int"}}, 
            {"kind": "CompoundStmt", "id": "0x6"}]}]}]}]})C";
        ASSERT_EQUALS("void foo ( ) { while ( 1 ) { } }", parse(clang_json));
    }


#define GET_SYMBOL_DB(AST) \
    const Settings settings = settingsBuilder().clang().platform(Platform::Type::Unix64).build(); \
    TokenList tokenlist{settings, Standards::Language::CPP}; \
    Tokenizer tokenizer(std::move(tokenlist), *this); \
    { \
        clangimport::parseClangAstDump(tokenizer, AST); \
    } \
    const SymbolDatabase *db = tokenizer.getSymbolDatabase(); \
    ASSERT(db)

    void tokenIndex() {
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "foo", "type": {"qualType": "void ()"}, "inner": [
            {"kind": "CompoundStmt", "id": "0x3"}]}]})C";

        ASSERT_EQUALS("void foo ( ) { }", parse(clang_json));

        GET_SYMBOL_DB(clang_json);
        const Token *tok = tokenizer.tokens();
        ASSERT_EQUALS(tok->index() + 1, tok->next()->index());
    }
/*
    void symbolDatabaseEnum1() {
        const char clang[] = "|-NamespaceDecl 0x29ad5f8 <1.cpp:1:1, line:3:1> line:1:11 ns\n"
                             "| `-EnumDecl 0x29ad660 <line:2:1, col:16> col:6 referenced abc\n"
                             "|   |-EnumConstantDecl 0x29ad720 <col:11> col:11 a 'ns::abc'\n"
                             "|   |-EnumConstantDecl 0x29ad768 <col:13> col:13 b 'ns::abc'\n"
                             "|   `-EnumConstantDecl 0x29ad7b0 <col:15> col:15 referenced c 'ns::abc'\n"
                             "`-VarDecl 0x29ad898 <line:5:1, col:22> col:9 x 'ns::abc':'ns::abc' cinit\n"
                             "  `-DeclRefExpr 0x29ad998 <col:13, col:22> 'ns::abc' EnumConstant 0x29ad7b0 'c' 'ns::abc'\n";

        ASSERT_EQUALS("namespace ns { enum abc { a , b , c } } ns :: abc x@1 = c ;", parse(clang));

        GET_SYMBOL_DB(clang);

        // Enum scope and type
        ASSERT_EQUALS(3, db->scopeList.size());
        const Scope &enumScope = db->scopeList.back();
        ASSERT_EQUALS_ENUM(ScopeType::eEnum, enumScope.type);
        ASSERT_EQUALS("abc", enumScope.className);
        const Type *enumType = enumScope.definedType;
        ASSERT_EQUALS("abc", enumType->name());

        // Variable
        const Token *vartok = Token::findsimplematch(tokenizer.tokens(), "x");
        ASSERT(vartok);
        ASSERT(vartok->variable());
        ASSERT(vartok->variable()->valueType());
        ASSERT_EQUALS(uintptr_t(&enumScope), uintptr_t(vartok->variable()->valueType()->typeScope));
    }
 */
    void symbolDatabaseFunction1() {
        // code: void foo (int x, int y) { }
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "foo", "type": {"qualType": "void (int, int)"}, "inner": [
            {"kind": "ParmVarDecl", "id": "0x3", "name": "x", "type": {"qualType": "int"}}, 
            {"kind": "ParmVarDecl", "id": "0x4", "name": "y", "type": {"qualType": "int"}}, 
            {"kind": "CompoundStmt", "id": "0x5"}]}]})C";
        GET_SYMBOL_DB(clang_json);

        // There is a function foo that has 2 arguments
        ASSERT_EQUALS(1, db->functionScopes.size());
        const Scope *scope = db->functionScopes[0];
        const Function *func = scope->function;
        ASSERT_EQUALS(2, func->argCount());
        ASSERT_EQUALS("x", func->getArgumentVar(0)->name());
        ASSERT_EQUALS("y", func->getArgumentVar(1)->name());
        ASSERT_EQUALS(ValueType::Type::INT, func->getArgumentVar(0)->valueType()->type);
        ASSERT_EQUALS(ValueType::Type::INT, func->getArgumentVar(1)->valueType()->type);
    }

    void symbolDatabaseFunction2() {
        // code: void foo (int, int) {}
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "foo", "type": {"qualType": "void (int, int)"}, "inner": [
            {"kind": "ParmVarDecl", "id": "0x3", "type": {"qualType": "int"}}, 
            {"kind": "ParmVarDecl", "id": "0x4", "type": {"qualType": "int"}}, 
            {"kind": "CompoundStmt", "id": "0x5"}]}]})C";

        GET_SYMBOL_DB(clang_json);

        // There is a function foo that has 2 arguments
        ASSERT_EQUALS(1, db->functionScopes.size());
        const Scope *scope = db->functionScopes[0];
        const Function *func = scope->function;
        ASSERT_EQUALS(2, func->argCount());
        ASSERT(!func->getArgumentVar(0)->nameToken());
        ASSERT(!func->getArgumentVar(1)->nameToken());
    }
/*
    void symbolDatabaseFunction3() { // #9640
        const char clang[] = "`-FunctionDecl 0x238fcd8 <9640.cpp:1:1, col:26> col:6 used bar 'bool (const char, int &)'\n"
                             "  |-ParmVarDecl 0x238fb10 <col:10, col:16> col:20 'const char'\n"
                             "  |-ParmVarDecl 0x238fbc0 <col:22, col:25> col:26 'int &'\n"
                             "  `-CompoundStmt 0x3d45c48 <col:12>\n";

        GET_SYMBOL_DB(clang);

        // There is a function foo that has 2 arguments
        ASSERT_EQUALS(1, db->functionScopes.size());
        const Scope *scope = db->functionScopes[0];
        const Function *func = scope->function;
        ASSERT_EQUALS(2, func->argCount());
        ASSERT_EQUALS(false, func->getArgumentVar(0)->isReference());
        ASSERT_EQUALS(true, func->getArgumentVar(1)->isReference());
    }

    void symbolDatabaseFunctionConst() {
        const char clang[] = "`-CXXRecordDecl 0x7e2d98 <1.cpp:2:1, line:5:1> line:2:7 class foo definition\n"
                             "  `-CXXMethodDecl 0x7e3000 <line:4:3, col:12> col:8 f 'void () const'";

        GET_SYMBOL_DB(clang);

        // There is a function f that is const
        ASSERT_EQUALS(2, db->scopeList.size());
        ASSERT_EQUALS(1, db->scopeList.back().functionList.size());
        const Function &func = db->scopeList.back().functionList.back();
        ASSERT(func.isConst());
    }

    void symbolDatabaseVariableRef() {
        const char clang[] = "`-FunctionDecl 0x1593df0 <3.cpp:1:1, line:4:1> line:1:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x15940b0 <col:12, line:4:1>\n"
                             "    |-DeclStmt 0x1593f58 <line:2:3, col:8>\n"
                             "    | `-VarDecl 0x1593ef0 <col:3, col:7> col:7 used x 'int'\n"
                             "    `-DeclStmt 0x1594098 <line:3:3, col:15>\n"
                             "      `-VarDecl 0x1593fb8 <col:3, col:14> col:8 ref 'int &' cinit\n"
                             "        `-DeclRefExpr 0x1594020 <col:14> 'int' lvalue Var 0x1593ef0 'x' 'int'";
        GET_SYMBOL_DB(clang);
        const Variable *refVar = db->variableList().back();
        ASSERT(refVar->isReference());
    }

    void symbolDatabaseVariableRRef() {
        const char clang[] = "`-FunctionDecl 0x1a40df0 <3.cpp:1:1, line:4:1> line:1:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x1a41180 <col:12, line:4:1>\n"
                             "    |-DeclStmt 0x1a40f58 <line:2:3, col:8>\n"
                             "    | `-VarDecl 0x1a40ef0 <col:3, col:7> col:7 used x 'int'\n"
                             "    `-DeclStmt 0x1a41168 <line:3:3, col:18>\n"
                             "      `-VarDecl 0x1a40fb8 <col:3, col:17> col:9 ref 'int &&' cinit\n"
                             "        `-ExprWithCleanups 0x1a410f8 <col:15, col:17> 'int' xvalue\n"
                             "          `-MaterializeTemporaryExpr 0x1a41098 <col:15, col:17> 'int' xvalue extended by Var 0x1a40fb8 'ref' 'int &&'\n"
                             "            `-BinaryOperator 0x1a41078 <col:15, col:17> 'int' '+'\n"
                             "              |-ImplicitCastExpr 0x1a41060 <col:15> 'int' <LValueToRValue>\n"
                             "              | `-DeclRefExpr 0x1a41020 <col:15> 'int' lvalue Var 0x1a40ef0 'x' 'int'\n"
                             "              `-IntegerLiteral 0x1a41040 <col:17> 'int' 1\n";

        ASSERT_EQUALS("void foo ( ) { int x@1 ; int && ref@2 = x@1 + 1 ; }", parse(clang));

        GET_SYMBOL_DB(clang);
        const Variable *refVar = db->variableList().back();
        ASSERT(refVar->isReference());
        ASSERT(refVar->isRValueReference());
    }

    void symbolDatabaseVariablePointerRef() {
        const char clang[] = "`-FunctionDecl 0x9b4f10 <3.cpp:1:1, col:17> col:6 used foo 'void (int *&)'\n"
                             "  `-ParmVarDecl 0x9b4e40 <col:10, col:16> col:16 p 'int *&'\n";

        ASSERT_EQUALS("void foo ( int * & p@1 ) ;", parse(clang));

        GET_SYMBOL_DB(clang);
        const Variable *p = db->variableList().back();
        ASSERT(p->isPointer());
        ASSERT(p->isReference());
    }
 */
    void symbolDatabaseNodeType1() {
        // code: long foo (long i) { return i + 1; }
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "foo", "type": {"qualType": "long (long)"}, "inner": [
            {"kind": "ParmVarDecl", "id": "0x3", "name": "i", "type": {"qualType": "long"}}, 
            {"kind": "CompoundStmt", "id": "0x4", "inner": [
            {"kind": "ReturnStmt", "id": "0x5", "inner": [
            {"kind": "BinaryOperator", "id": "0x6", "opcode": "+", "type": {"qualType": "long"}, "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x7", "type": {"qualType": "long"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0x8", "type": {"qualType": "long"}, "referencedDecl": {"name": "i", "id": "0x3"}}]}, 
            {"kind": "ImplicitCastExpr", "id": "0x9", "type": {"qualType": "long"}, "inner": [
            {"kind": "IntegerLiteral", "id": "0xa", "value": "1", "type": {"qualType": "int"}}]}]}]}]}]}]})C";

        GET_SYMBOL_DB(clang_json);

        const Token *tok = Token::findsimplematch(tokenizer.tokens(), "i + 1");
        ASSERT(!!tok);
        ASSERT(!!tok->valueType());
        ASSERT_EQUALS("signed long", tok->valueType()->str());
    }

    void symbolDatabaseForVariable() {
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "FunctionDecl", "id": "0x2", "name": "foo", "type": {"qualType": "void ()"}, "inner": [
            {"kind": "CompoundStmt", "id": "0x3", "inner": [
            {"kind": "ForStmt", "id": "0x4", "inner": [
            {"kind": "DeclStmt", "id": "0x5", "inner": [
            {"kind": "VarDecl", "id": "0x6", "name": "i", "init": "c", "type": {"qualType": "int"}, "inner": [
            {"kind": "IntegerLiteral", "id": "0x7", "value": "0", "type": {"qualType": "int"}}]}]}, {}, {}, {}, 
            {"kind": "CompoundStmt", "id": "0x8"}]}]}]}]})C";

        ASSERT_EQUALS("void foo ( ) { for ( int i@1 = 0 ; ; ) { } }", parse(clang_json));

        GET_SYMBOL_DB(clang_json);

        const Token *tok = Token::findsimplematch(tokenizer.tokens(), "i");
        ASSERT(!!tok);
        ASSERT(!!tok->variable());
        ASSERT_EQUALS_ENUM(ScopeType::eFor, tok->variable()->scope()->type);
    }

    void valueFlow1() {
        // struct S { int x; int buf[10]; } ; int sz = sizeof(struct S);
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "RecordDecl", "id": "0x2", "name": "S", "completeDefinition": true, "tagUsed": "struct", "inner": [
            {"kind": "FieldDecl", "id": "0x3", "name": "x", "type": {"qualType": "int"}}, 
            {"kind": "FieldDecl", "id": "0x4", "name": "buf", "type": {"qualType": "int[10]"}}]}, 
            {"kind": "VarDecl", "id": "0x5", "name": "sz", "init": "c", "type": {"qualType": "int"}, "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x6", "type": {"qualType": "int"}, "inner": [
            {"kind": "UnaryExprOrTypeTraitExpr", "id": "0x7", "name": "sizeof", "type": {"qualType": "unsigned long"}, "argType": {"qualType": "struct S"}}]}]}]})C";

        GET_SYMBOL_DB(clang_json);

        const Token *tok = Token::findsimplematch(tokenizer.tokens(), "sizeof (");
        ASSERT(!!tok);
        tok = tok->next();
        ASSERT(tok->hasKnownIntValue());
        ASSERT_EQUALS(44, tok->getKnownIntValue());
    }

    void valueFlow2() {
        // int buf[42]; int x = sizeof(buf);
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "VarDecl", "id": "0x2", "name": "buf", "type": {"qualType": "int[42]"}}, 
            {"kind": "VarDecl", "id": "0x3", "name": "x", "init": "c", "type": {"qualType": "int"}, "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x4", "type": {"qualType": "int"}, "inner": [
            {"kind": "UnaryExprOrTypeTraitExpr", "id": "0x5", "name": "sizeof", "type": {"qualType": "unsigned long"}, "inner": [
            {"kind": "ParenExpr", "id": "0x6", "type": {"qualType": "int[42]"}, "inner": [
            {"kind": "DeclRefExpr", "id": "0x7", "type": {"qualType": "int[42]"}, "referencedDecl": {"name": "buf", "id": "0x2"}}]}]}]}]}]})C";

        GET_SYMBOL_DB(clang_json);

        const Token *tok = Token::findsimplematch(tokenizer.tokens(), "sizeof (");
        ASSERT(!!tok);
        tok = tok->next();
        TODO_ASSERT_EQUALS(true, false, tok->hasKnownIntValue() && tok->getKnownIntValue() == 10);
    }
/*
    void valueType1() {
        const char clang[] = "`-FunctionDecl 0x32438c0 <a.cpp:5:1, line:7:1> line:5:6 foo 'a::b (a::b)'\n"
                             "  |-ParmVarDecl 0x32437b0 <col:10, col:15> col:15 used i 'a::b':'long'\n"
                             "  `-CompoundStmt 0x3243a60 <col:18, line:7:1>\n"
                             "    `-ReturnStmt 0x3243a48 <line:6:3, col:12>\n"
                             "      `-ImplicitCastExpr 0x2176ca8 <col:9> 'int' <IntegralCast>\n"
                             "        `-ImplicitCastExpr 0x2176c90 <col:9> 'bool' <LValueToRValue>\n"
                             "          `-DeclRefExpr 0x2176c60 <col:9> 'bool' lvalue Var 0x2176bd0 'e' 'bool'\n";

        GET_SYMBOL_DB(clang);

        const Token *tok = Token::findsimplematch(tokenizer.tokens(), "e");
        ASSERT(!!tok);
        ASSERT(!!tok->valueType());
        ASSERT_EQUALS("bool", tok->valueType()->str());
    }
 */
    void valueType2() {
        // const char* s = "hello";
        const char clang_json[] = R"C(
            {"kind": "TranslationUnitDecl", "id": "0x1", "inner": [
            {"kind": "VarDecl", "id": "0x2", "name": "s", "init": "c", "type": {"qualType": "const char *"}, "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x3", "type": {"qualType": "const char *"}, "inner": [
            {"kind": "ImplicitCastExpr", "id": "0x4", "type": {"qualType": "char *"}, "inner": [
            {"kind": "StringLiteral", "id": "0x5", "value": "\"hello\"", "type": {"qualType": "char[6]"}}]}]}]}]})C";

        ASSERT_EQUALS("const char * s@1 = \"hello\" ;", parse(clang_json));

        GET_SYMBOL_DB(clang_json);

        const Token *tok = Token::findsimplematch(tokenizer.tokens(), "\"hello\"");
        ASSERT(!!tok);
        ASSERT(!!tok->valueType());
        ASSERT_EQUALS("const signed char *", tok->valueType()->str());
    }
};

REGISTER_TEST(TestClangImport)
