//---------------------------------------------------------------------------
#include "CheckHeaders.h"
#include "Tokenize.h"
#include "CommonCheck.h"
#include <sstream>
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// HEADERS - No implementation in a header
//---------------------------------------------------------------------------

void WarningHeaderWithImplementation()
{
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        // Only interested in included file
        if (tok->FileIndex == 0)
            continue;

        if (match(tok, ") {"))
        {
            std::ostringstream ostr;
            ostr << FileLine(tok) << ": Found implementation in header";
            ReportErr(ostr.str());
        }
    }
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
// HEADERS - Unneeded include
//---------------------------------------------------------------------------

void WarningIncludeHeader()
{
    // Including..
    for (TOKEN *includetok = tokens; includetok; includetok = includetok->next)
    {
        if (strcmp(includetok->str, "#include") != 0)
            continue;

        // Get fileindex of included file..
        unsigned int hfile = 0;
        const char *includefile = includetok->next->str;
        while (hfile < Files.size())
        {
            if (stricmp(Files[hfile].c_str(), includefile) == 0)
                break;
            hfile++;
        }
        if (hfile == Files.size())
            continue;

        // This header is needed if:
        // * It contains some needed class declaration
        // * It contains some needed function declaration
        // * It contains some needed constant value
        // * It contains some needed variable
        // * It contains some needed enum
        bool Needed = false;
        bool NeedDeclaration = false;
        int indentlevel = 0;
        for (TOKEN *tok1 = tokens; tok1; tok1 = tok1->next)
        {
            if (tok1->FileIndex != hfile)
                continue;

            if (!tok1->next)
                break;

            if (!tok1->next->next)
                break;

            // I'm only interested in stuff that is declared at indentlevel 0
            if (tok1->str[0] == '{')
                indentlevel++;
            if (tok1->str[0] == '}')
                indentlevel--;
            if (indentlevel != 0)
                continue;

            // Class or namespace declaration..
            if (match(tok1,"class var {") ||
                match(tok1,"class var :") ||
                match(tok1,"namespace var {"))
            {
                std::string classname = getstr(tok1, 1);

                // Try to find class usage in "parent" file..
                for (TOKEN *tok2 = tokens; tok2; tok2 = tok2->next)
                {
                    if (tok2->FileIndex != includetok->FileIndex)
                        continue;

                    // Inheritage..
                    Needed |= match(tok2, "class var : " + classname);
                    Needed |= match(tok2, "class var : type " + classname);

                    // Allocating..
                    Needed |= match(tok2, "new " + classname);

                    // Using class..
                    Needed |= match(tok2, classname + " ::");
                    Needed |= match(tok2, classname + " var");
                    NeedDeclaration |= match(tok2, classname + " *");
                    if (Needed | NeedDeclaration)
                        break;
                }

                if (Needed | NeedDeclaration)
                    break;
            }

            // Variable..
            std::string varname = "";
            if (match(tok1, "type var ;") || match(tok1, "type var ["))
                varname = getstr(tok1, 1);
            if (match(tok1, "type * var ;") || match(tok1, "type * var ["))
                varname = getstr(tok1, 2);
            if (match(tok1, "const type var =") || match(tok1, "const type var ["))
                varname = getstr(tok1, 2);
            if (match(tok1, "const type * var =") || match(tok1, "const type * var ["))
                varname = getstr(tok1, 3);

            // enum..
            std::string enumname = "";
            if (match(tok1, "enum var {"))
                enumname = getstr(tok1, 1);

            // function..
            std::string funcname = "";
            if (match(tok1,"type var ("))
                funcname = getstr(tok1, 1);
            else if (match(tok1,"type * var ("))
                funcname = getstr(tok1, 2);
            else if (match(tok1,"const type var ("))
                funcname = getstr(tok1, 2);
            else if (match(tok1,"const type * var ("))
                funcname = getstr(tok1, 3);

            // typedef..
            std::string typedefname = "";
            if (strcmp(tok1->str,"typedef")==0)
            {
                int parlevel = 0;
                while (tok1)
                {
                    if ( strchr("({", tok1->str[0]) )
                    {
                        parlevel++;
                    }
                    else if ( strchr("({", tok1->str[0]) )
                    {
                        parlevel--;
                        if (parlevel < 0)
                            break;
                    }
                    else if ( parlevel == 0 )
                    {
                        if (match(tok1,"var ;"))
                        {
                            typedefname = tok1->str;
                            break;
                        }
                        if (tok1->str[0] == ';')
                        {
                            break;
                        }
                    }

                    tok1 = tok1->next;
                }
            }


            if ( varname.empty() && enumname.empty() && funcname.empty() && typedefname.empty() )
                continue;

            varname = varname + enumname + funcname + typedefname;

            for (TOKEN *tok2 = tokens; tok2; tok2 = tok2->next)
            {
                if (tok2->FileIndex == includetok->FileIndex &&
                    tok2->str == varname)
                {
                    Needed = true;
                    break;
                }
            }

            if (Needed | NeedDeclaration)
                break;
        }


        // Not a header file?
        if (includetok->FileIndex == 0)
            Needed |= NeedDeclaration;

        // Not needed!
        if (!Needed)
        {
            std::ostringstream ostr;
            ostr << FileLine(includetok) << ": The included header '" << includefile << "' is not needed";
            if (NeedDeclaration)
                ostr << " (but a declaration in it is needed)";
            ReportErr(ostr.str());
        }
    }
}
//---------------------------------------------------------------------------



