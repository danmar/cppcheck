
#include "CheckMemoryLeak.h"

#include "Tokenize.h"
#include "Statements.h"

#include "CommonCheck.h"

#include <vector>
#include <sstream>

#include <mem.h>     // <- memset

//---------------------------------------------------------------------------

struct _variable
{
    int indentlevel;
    unsigned int varindex;
    enum {Any, Data, Malloc, New, NewA} value;
};

//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Checks for memory leaks inside function..
//---------------------------------------------------------------------------

static void _InFunction()
{
    std::vector<_variable *> varlist;

    // if
    int iflevel = 0;
    bool endif = false;
    std::vector<bool> iflist;

    // loop
    int looplevel = 0;
    bool endloop = false;
    std::vector<bool> looplist;

    // switch
    int switchlevel = 0;
    bool endswitch = false;
    std::vector<bool> switchlist;

    // Parse the statement list and locate memory leaks..
    int indentlevel = 0;
    std::list<STATEMENT>::const_iterator it;
    for (it = Statements.begin(); it != Statements.end(); it++)
    {
        switch (it->Type)
        {
            case STATEMENT::OBRACE:
                indentlevel++;
                iflist.push_back(endif);
                if (endif)
                    iflevel++;
                looplist.push_back(endloop);
                if (endloop)
                    looplevel++;
                switchlist.push_back(endswitch);
                if (endswitch)
                    switchlevel++;
                break;

            case STATEMENT::EBRACE:
                // Check if any variables go out of scope..
                if (indentlevel >= 1)
                {
                    for (unsigned int i = 0; i < varlist.size(); i++)
                    {
                        if (varlist[i]->indentlevel != indentlevel)
                            continue;

                        if (varlist[i]->value == _variable::Malloc || varlist[i]->value == _variable::New || varlist[i]->value == _variable::NewA)
                        {
                            std::ostringstream ostr;
                            ostr << FileLine(it->Token) << ": Memory leak:" << VariableNames[varlist[i]->varindex];
                            ReportErr(ostr.str());
                        }

                        // Delete this instance..
                        delete varlist[i];
                        varlist.erase(varlist.begin()+i);
                    }
                }

                // if level..
                if ( ! iflist.empty() )
                {
                    if (iflist.back())
                        iflevel--;
                    iflist.pop_back();
                }

                // loop level..
                if ( ! looplist.empty() )
                {
                    if (looplist.back())
                        looplevel--;
                    looplist.pop_back();
                }

                // switch level..
                if ( ! switchlist.empty() )
                {
                    if (switchlist.back())
                        switchlevel--;
                    switchlist.pop_back();
                }

                // Make sure the varlist is empty..
                if (indentlevel <= 1)
                    varlist.clear();

                if (indentlevel > 0)
                    indentlevel--;
                break;

            case STATEMENT::DECL:
                if (indentlevel > 0)
                {
                    _variable *NewVar = new _variable;
                    memset(NewVar, 0, sizeof(_variable));
                    NewVar->indentlevel = indentlevel;
                    NewVar->varindex = it->VarIndex;
                    varlist.push_back(NewVar);
                }
                break;

            case STATEMENT::IF:
            case STATEMENT::ELSEIF:
            case STATEMENT::ELSE:
                // Conditional statements..
                iflevel++;
                break;

            case STATEMENT::ENDIF:
                iflevel--;
                break;

            // Not very interested in these..
            case STATEMENT::LOOP:
            case STATEMENT::ENDLOOP:
            case STATEMENT::SWITCH:
            case STATEMENT::ENDSWITCH:
                break;

            case STATEMENT::MALLOC:
            case STATEMENT::NEW:
            case STATEMENT::NEWARRAY:
            case STATEMENT::FREE:
            case STATEMENT::DELETE:
            case STATEMENT::DELETEARRAY:
                // Don't handle conditional allocation at the moment..
                if (iflevel>0 && (it->Type==STATEMENT::MALLOC || it->Type==STATEMENT::NEW || it->Type==STATEMENT::NEWARRAY))
                    break;

                for (unsigned int i = 0; i < varlist.size(); i++)
                {
                    if ( varlist[i]->varindex == it->VarIndex )
                    {
                        bool isalloc = (varlist[i]->value==_variable::Malloc || varlist[i]->value==_variable::New || varlist[i]->value==_variable::NewA);
                        bool alloc = (it->Type==STATEMENT::MALLOC || it->Type==STATEMENT::NEW || it->Type==STATEMENT::NEWARRAY);

                        // Already allocated and then allocated again..
                        //bool err = (isalloc & alloc);

                        if (isalloc && !alloc)
                        {
                            // Check that the deallocation matches..
                            bool err = false;

                            err |= (varlist[i]->value==_variable::Malloc && it->Type!=STATEMENT::FREE);
                            err |= (varlist[i]->value==_variable::New    && it->Type!=STATEMENT::DELETE);
                            err |= (varlist[i]->value==_variable::NewA   && it->Type!=STATEMENT::DELETEARRAY);

                            if (err)
                            {
                                std::ostringstream ostr;
                                ostr << FileLine(it->Token) << ": Mismatching allocation and deallocation '" << VariableNames[varlist[i]->varindex] << "'";
                                ReportErr(ostr.str());
                            }
                        }

                        if ( it->Type == STATEMENT::MALLOC )
                            varlist[i]->value = _variable::Malloc;

                        else if ( it->Type == STATEMENT::FREE )
                            varlist[i]->value = _variable::Any;

                        else if ( it->Type == STATEMENT::NEW )
                            varlist[i]->value = _variable::New;

                        else if ( it->Type == STATEMENT::NEWARRAY )
                            varlist[i]->value = _variable::NewA;

                        else if ( it->Type == STATEMENT::DELETE )
                            varlist[i]->value = _variable::Any;

                        else if ( it->Type == STATEMENT::DELETEARRAY )
                            varlist[i]->value = _variable::Any;

                        break;
                    }
                }
                break;

            case STATEMENT::ASSIGN:
            case STATEMENT::USE:
                for (unsigned int i = 0; i < varlist.size(); i++)
                {
                    if ( varlist[i]->varindex == it->VarIndex )
                    {
                        varlist[i]->value = _variable::Data;
                        break;
                    }
                }
                break;

            case STATEMENT::RETURN:
                for (unsigned int i = 0; i < varlist.size(); i++)
                {
                    if ( varlist[i]->varindex == it->VarIndex )
                    {
                        varlist[i]->value = _variable::Any;
                    }

                    else if (varlist[i]->value==_variable::New ||
                             varlist[i]->value==_variable::NewA )
                    {
                        std::ostringstream ostr;
                        ostr << FileLine(it->Token) << ": Memory leak:" << VariableNames[varlist[i]->varindex];
                        ReportErr(ostr.str());
                        varlist[i]->value = _variable::Any;
                        break;
                    }
                }
                break;

            case STATEMENT::CONTINUE:
            case STATEMENT::BREAK:
                // Memory leak if variable is allocated..
                if (looplevel>0)
                {
                    // Find the last loop..
                    for (int i = looplist.size() - 1; i >= 0; i--)
                    {
                        if (switchlist[i])
                            break;

                        if (!looplist[i])
                            continue;

                        int loop_indentlevel = i + 1;

                        for (i = 0; i < (int)varlist.size(); i++)
                        {
                            if (varlist[i]->indentlevel < loop_indentlevel)
                                continue;

                            if (varlist[i]->value==_variable::Malloc ||
                                varlist[i]->value==_variable::New ||
                                varlist[i]->value==_variable::NewA )
                            {
                                std::ostringstream ostr;
                                ostr << FileLine(it->Token) << ": Memory leak:" << VariableNames[varlist[i]->varindex];
                                ReportErr(ostr.str());
                            }

                            break;
                        }

                        break;
                    }
                }
                break;
        }

        endif = (it->Type == STATEMENT::ENDIF);
        endloop = (it->Type == STATEMENT::ENDLOOP);
        endswitch = (it->Type == STATEMENT::ENDSWITCH);
    }
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Check that all class members are deallocated..
//---------------------------------------------------------------------------

static void _ClassMembers_CheckVar(const char *classname, const char *varname)
{
    // Check how the variable 'varname' is allocated..
    enum {No, New, NewA, Malloc} Alloc, Dealloc;
    Alloc = Dealloc = No;
    for (TOKEN *func = tokens; func; func = func->next)
    {
        if (strcmp(func->str, classname))
            continue;

        if (strcmp(getstr(func, 1), "::") != 0)
            continue;

        if (strcmp(getstr(func, 3), "(") != 0)
            continue;


        int indentlevel = 0;
        for (TOKEN *tok = func; tok; tok = tok->next)
        {
            if (tok->str[0] == '{')
            {
                indentlevel++;
            }

            else if (tok->str[0] == '}')
            {
                indentlevel--;
                if (indentlevel == 0)
                {
                    func = tok;
                    break;
                }
            }

            else if (indentlevel > 0)
            {
                // Deallocation..
                if (strcmp(tok->str,"delete")==0 || strcmp(tok->str,"free")==0)
                {
                    bool err = false;

                    if (match(tok, "delete var ;") &&
                        strcmp(getstr(tok,1),varname)==0)
                    {
                        err |= ( Dealloc != No && Dealloc != New );
                        Dealloc = New;
                    }

                    else if (match(tok, "delete [ ] var ;") &&
                        strcmp(getstr(tok,3),varname)==0)
                    {
                        err |= ( Dealloc != No && Dealloc != NewA );
                        Dealloc = NewA;
                    }

                    else if (match(tok, "free ( var )") &&
                        strcmp(getstr(tok,2),varname)==0)
                    {
                        err |= ( Dealloc != No && Dealloc != Malloc );
                        Dealloc = Malloc;
                    }

                    if (err)
                    {
                        std::ostringstream ostr;
                        ostr << FileLine(tok) << ": Mismatching deallocation for '" << classname << "::" << varname << "'";
                        ReportErr(ostr.str());
                    }
                }

                else
                {
                    // Allocation...
                    if ( strcmp(tok->str, varname) != 0 )
                        continue;

                    if ( strcmp(getstr(tok,1), "=") != 0 )
                        continue;

                    bool err = false;

                    if ( strcmp(getstr(tok,2), "new") == 0 )
                    {
                        if ( match(tok, "var = new type ;") ||
                             match(tok, "var = new type (") )
                        {
                            err |= ( Alloc != No && Alloc != New );
                            Alloc = New;
                        }

                        else if ( match(tok, "var = new type [") )
                        {
                            err |= ( Alloc != No && Alloc != NewA );
                            Alloc = NewA;
                        }
                    }

                    else if ( match(tok, "var = strdup (") ||
                              match(tok, "var = ( type * ) malloc ("))
                    {
                        err |= ( Alloc != No && Alloc != Malloc );
                        Alloc = Malloc;
                    }

                    if (err)
                    {
                        std::ostringstream ostr;
                        ostr << FileLine(tok) << ": Mismatching allocation for '" << classname << "::" << varname << "'";
                        ReportErr(ostr.str());
                    }
                }
            }
        }
    }

    if ( Alloc != Dealloc )
    {
        if ( Dealloc == No )
        {
            std::ostringstream ostr;
            ostr << "Memory leak for '" << classname << "::" << varname << "'";
            ReportErr(ostr.str());
        }
    }

}

static void _ClassMembers()
{
    for (TOKEN *ClassDecl = tokens; ClassDecl; ClassDecl = ClassDecl->next)
    {
        // Is this a class declaration?
        // -------------------------------------
        if ( ! match(ClassDecl, "class var {") )
            continue;

        const char *classname = getstr(ClassDecl, 1);

        // Locate member variables..
        int indentlevel = 0;
        for (TOKEN *ClassVar = ClassDecl; ClassVar; ClassVar = ClassVar->next)
        {
            if ( ClassVar->str[0] == '{' )
                indentlevel++;

            else if ( ClassVar->str[0] == '}' )
            {
                indentlevel--;
                if (indentlevel == 0)
                    break;
            }

            else if (indentlevel == 1)
            {
                if ( match(ClassVar, "type * var ;") )
                {
                    const char *varname = getstr(ClassVar, 2);

                    // Check the variable usage..
                    _ClassMembers_CheckVar(classname, varname);
                }
            }
        }
    }
}


//---------------------------------------------------------------------------
// Checks for memory leaks..
//---------------------------------------------------------------------------

void CheckMemoryLeak()
{
    // Check for memory leaks inside functions..
    _InFunction();

    // Check that all class members are deallocated..
    _ClassMembers();
}
//---------------------------------------------------------------------------




