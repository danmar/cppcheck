//---------------------------------------------------------------------------
#ifndef StatementsH
#define StatementsH
//---------------------------------------------------------------------------

#include <list>
#include <vector>
#include <string>

extern std::vector<std::string> VariableNames;

// Forward declaration
struct TOKEN;

struct STATEMENT
{
    enum etype {OBRACE, EBRACE,
                DECL, ASSIGN, USE,
                MALLOC, FREE,
                NEW, DELETE,
                NEWARRAY, DELETEARRAY,
                LOOP, ENDLOOP,
                IF, ELSE, ELSEIF, ENDIF,
                SWITCH, ENDSWITCH,
                RETURN, CONTINUE, BREAK};
    etype Type;
    unsigned int VarIndex;
    TOKEN *Token;
};

extern std::list<STATEMENT> Statements;

void CreateStatementList();

void OutputStatementList( std::ostream &ostr );

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
