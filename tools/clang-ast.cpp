// Dump the AST for a file.
//
// Compile with:
// g++ `llvm-config-3.8 --cxxflags --ldflags` -lclang -o clang-ast clang-ast.cpp

#include <iostream>
#include <cstdlib>
#include <clang-c/Index.h>

std::ostream& operator<<(std::ostream& stream, const CXString& str)
{
    stream << clang_getCString(str);
    clang_disposeString(str);
    return stream;
}

int main(int argc, char **argv)
{
    if (argc == 1) {
        std::cerr << "No source file\n";
        return EXIT_FAILURE;
    }

    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit unit = clang_parseTranslationUnit(
                                 index,
                                 argv[1], nullptr, 0,
                                 nullptr, 0,
                                 CXTranslationUnit_None);
    if (unit == nullptr) {
        std::cerr << "Unable to parse translation unit. Quitting." << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "<?xml version=\"1.0\"?>\n"
              << "<clang-ast>\n";

    CXCursor cursor = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(
        cursor,
    [](CXCursor c, CXCursor parent, CXClientData client_data) {
        switch (clang_getCursorKind(c)) {
        case CXCursor_FunctionDecl:
        case CXCursor_CXXMethod: {
            CXSourceLocation location = clang_getCursorLocation(c);
            CXString filename;
            unsigned int line, column;
            clang_getPresumedLocation(location, &filename, &line, &column);

            std::cout << "<" << clang_getCursorKindSpelling(clang_getCursorKind(c))
                      << " name=\"" << clang_getCursorSpelling(c) << '\"'
                      << " filename=\"" << filename << '\"'
                      << " line=\"" << line << '\"'
//                        << " column=\"" << column << '\"'
                      << "/>\n";
            break;
        }

        case CXCursor_CallExpr: {
            CXSourceLocation location = clang_getCursorLocation(c);
            CXString filename;
            unsigned int line, column;
            clang_getPresumedLocation(location, &filename, &line, &column);

            CXCursor ref = clang_getCursorReferenced(c);
            CXSourceLocation locationRef = clang_getCursorLocation(ref);
            CXString filenameRef;
            unsigned int lineRef, columnRef;
            clang_getPresumedLocation(locationRef, &filenameRef, &lineRef, &columnRef);

            std::cout << "<CallExpr"
                      << " name=\"" << clang_getCursorSpelling(c) << '\"'
                      << " filename=\"" << filename << '\"'
                      << " line=\"" << line << '\"'
//                        << " column=\"" << column << '\"'
                      << " filenameRef=\"" << filenameRef << '\"'
                      << " lineRef=\"" << lineRef << '\"'
                      << "/>\n";
            break;
        }
        break;
        default:
            break;
        };

        return CXChildVisit_Recurse;
    },
    nullptr);

    std::cout << "</clang-ast>\n";

    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);

    return EXIT_SUCCESS;
}
