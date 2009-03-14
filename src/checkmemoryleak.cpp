/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis, Vesa Pikki
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */


#include "checkmemoryleak.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

//---------------------------------------------------------------------------

CheckMemoryLeakClass::CheckMemoryLeakClass(const Tokenizer *tokenizer, const Settings &settings, ErrorLogger *errorLogger)
        : _settings(settings)
{
    _tokenizer = tokenizer;
    _errorLogger = errorLogger;
}

CheckMemoryLeakClass::~CheckMemoryLeakClass()
{

}

bool CheckMemoryLeakClass::isclass(const Token *tok)
{
    if (tok->isStandardType())
        return false;

    std::ostringstream pattern;
    pattern << "struct " << tok->str();
    if (Token::findmatch(_tokenizer->tokens(), pattern.str().c_str()))
        return false;

    return true;
}
//---------------------------------------------------------------------------

CheckMemoryLeakClass::AllocType CheckMemoryLeakClass::GetAllocationType(const Token *tok2)
{
    // What we may have...
    //     * var = (char *)malloc(10);
    //     * var = new char[10];
    //     * var = strdup("hello");
    if (tok2 && tok2->str() == "(")
    {
        while (tok2 && tok2->str() != ")")
            tok2 = tok2->next();
        tok2 = tok2 ? tok2->next() : NULL;
    }
    if (! tok2)
        return No;
    if (! tok2->isName())
        return No;

    // Does tok2 point on "malloc", "strdup" or "kmalloc"..
    const char *mallocfunc[] = {"malloc",
                                "calloc",
                                "strdup",
                                "kmalloc",
                                "kzalloc",
                                "kcalloc",
                                0
                               };
    for (unsigned int i = 0; mallocfunc[i]; i++)
    {
        if (tok2->str() == mallocfunc[i])
            return Malloc;
    }

    // Does tok2 point on "g_malloc", "g_strdup", ..
    const char *gmallocfunc[] = {"g_new",
                                 "g_new0",
                                 "g_try_new",
                                 "g_try_new0",
                                 "g_malloc",
                                 "g_malloc0",
                                 "g_try_malloc",
                                 "g_try_malloc0",
                                 "g_strdup",
                                 "g_strndup",
                                 0
                                };
    for (unsigned int i = 0; gmallocfunc[i]; i++)
    {
        if (tok2->str() == gmallocfunc[i])
            return gMalloc;
    }

    if (Token::Match(tok2, "new %type% [;(]"))
        return New;

    if (Token::Match(tok2, "new %type% ["))
        return NewArray;

    if (Token::Match(tok2, "fopen ("))
        return FOPEN;

    if (Token::Match(tok2, "popen ("))
        return POPEN;

    // Userdefined allocation function..
    std::list<AllocFunc>::const_iterator it = _listAllocFunc.begin();
    while (it != _listAllocFunc.end())
    {
        if (tok2->str() == it->funcname)
            return it->alloctype;
        ++it;
    }

    return No;
}



CheckMemoryLeakClass::AllocType CheckMemoryLeakClass::GetReallocationType(const Token *tok2)
{
    // What we may have...
    //     * var = (char *)realloc(..;
    if (tok2 && tok2->str() == "(")
    {
        while (tok2 && tok2->str() != ")")
            tok2 = tok2->next();
        tok2 = tok2 ? tok2->next() : NULL;
    }
    if (! tok2)
        return No;

    if (Token::Match(tok2, "realloc"))
        return Malloc;

    // GTK memory reallocation..
    if (Token::Match(tok2, "g_realloc|g_try_realloc|g_renew|g_try_renew"))
        return gMalloc;

    return No;
}


CheckMemoryLeakClass::AllocType CheckMemoryLeakClass::GetDeallocationType(const Token *tok, const char *varnames[])
{
    int i = 0;
    std::string names;
    while (varnames[i])
    {
        if (i > 0)
            names += " . ";

        names += varnames[i];
        i++;
    }

    if (Token::simpleMatch(tok, std::string("delete " + names + " ;").c_str()))
        return New;

    if (Token::simpleMatch(tok, std::string("delete [ ] " + names + " ;").c_str()))
        return NewArray;

    if (Token::simpleMatch(tok, std::string("delete ( " + names + " ) ;").c_str()))
        return New;

    if (Token::simpleMatch(tok, std::string("delete [ ] ( " + names + " ) ;").c_str()))
        return NewArray;

    if (Token::simpleMatch(tok, std::string("free ( " + names + " ) ;").c_str()) ||
        Token::simpleMatch(tok, std::string("kfree ( " + names + " ) ;").c_str()))
    {
        return Malloc;
    }

    if (Token::simpleMatch(tok, std::string("g_free ( " + names + " ) ;").c_str()))
        return gMalloc;

    if (Token::simpleMatch(tok, std::string("fclose ( " + names + " )").c_str()))
        return FOPEN;

    if (Token::simpleMatch(tok, std::string("pclose ( " + names + " )").c_str()))
        return POPEN;

    return No;
}
//--------------------------------------------------------------------------

const char * CheckMemoryLeakClass::call_func(const Token *tok, std::list<const Token *> callstack, const char *varnames[], AllocType &alloctype, AllocType &dealloctype, bool &all, unsigned int sz)
{
    // Keywords that are not function calls..
    if (Token::Match(tok, "if|for|while|return|switch"))
        return 0;

    // String functions that are not allocating nor deallocating memory..
    if (Token::Match(tok, "strcpy|strncpy|strcat|strncat|strcmp|strncmp|strcasecmp|stricmp|sprintf|strchr|strrchr|strstr"))
        return 0;

    // Memory functions that are not allocating nor deallocating memory..
    if (Token::Match(tok, "memset|memcpy|memmove|memchr"))
        return 0;

    // I/O functions that are not allocating nor deallocating memory..
    if (Token::Match(tok, "fgets|fgetc|fputs|fputc|printf"))
        return 0;

    // Convert functions that are not allocating nor deallocating memory..
    if (Token::Match(tok, "atoi|atof|atol|strtol|strtoul|strtod"))
        return 0;

    // This is not an unknown function neither
    if (tok->str() == "delete")
        return 0;

    if (GetAllocationType(tok) != No || GetReallocationType(tok) != No || GetDeallocationType(tok, varnames) != No)
        return 0;

    if (callstack.size() > 2)
        return "dealloc_";

    const std::string funcname(tok->str());
    for (std::list<const Token *>::const_iterator it = callstack.begin(); it != callstack.end(); ++it)
    {
        if ((*it)->str() == funcname)
            return "recursive";
    }
    callstack.push_back(tok);

    int par = 1;
    int parlevel = 0;
    std::string pattern = "[,()] ";
    for (int i = 0; varnames[i]; i++)
    {
        if (i > 0)
            pattern += " . ";

        pattern += varnames[i];
    }

    pattern += " [,()]";

    for (; tok; tok = tok->next())
    {
        if (tok->str() == "(")
            ++parlevel;
        else if (tok->str() == ")")
        {
            --parlevel;
            if (parlevel < 1)
            {
                return _settings._showAll ? 0 : "callfunc";
            }
        }

        if (parlevel == 1)
        {
            if (tok->str() == ",")
                ++par;
            if (Token::Match(tok, pattern.c_str()))
            {
                const Token *ftok = _tokenizer->GetFunctionTokenByName(funcname.c_str());
                const char *parname = Tokenizer::getParameterName(ftok, par);
                if (! parname)
                    return "recursive";
                // Check if the function deallocates the variable..
                while (ftok && (ftok->str() != "{"))
                    ftok = ftok->next();
                Token *func = getcode(ftok->tokAt(1), callstack, parname, alloctype, dealloctype, false, all, sz);
                simplifycode(func, all);
                const Token *func_ = func;
                while (func_ && func_->str() == ";")
                    func_ = func_->next();

                const char *ret = 0;
                // TODO : "goto" isn't handled well
                if (Token::findmatch(func_, "dealloc"))
                    ret = "dealloc";
                else if (Token::findmatch(func_, "use"))
                    ret = "use";
                else if (Token::findmatch(func_, "&use"))
                    ret = "&use";

                Tokenizer::deleteTokens(func);
                return ret;
            }
        }
    }
    return NULL;
}

//--------------------------------------------------------------------------

void CheckMemoryLeakClass::MemoryLeak(const Token *tok, const char varname[], AllocType alloctype, bool all)
{
    if (alloctype == CheckMemoryLeakClass::FOPEN ||
        alloctype == CheckMemoryLeakClass::POPEN)
        _errorLogger->resourceLeak(_tokenizer, tok, varname);
    else if (all)
        _errorLogger->memleakall(_tokenizer, tok, varname);
    else
        _errorLogger->memleak(_tokenizer, tok, varname);
}
//---------------------------------------------------------------------------

bool CheckMemoryLeakClass::MatchFunctionsThatReturnArg(const Token *tok, const std::string &varname)
{
    return Token::Match(tok, std::string("; " + varname + " = strcat|memcpy|memmove|strcpy ( " + varname + " ,").c_str());
}

bool CheckMemoryLeakClass::notvar(const Token *tok, const char *varnames[], bool endpar)
{
    std::string varname;
    for (int i = 0; varnames[i]; i++)
    {
        if (i > 0)
            varname += " . ";

        varname += varnames[i];
    }

    const std::string end(endpar ? " )" : " [;)&|]");

    return bool(Token::Match(tok, std::string("! " + varname + end).c_str()) ||
                Token::simpleMatch(tok, std::string("! ( " + varname + " )" + end).c_str()) ||
                Token::Match(tok, std::string("0 == " + varname + end).c_str()) ||
                Token::simpleMatch(tok, std::string(varname + " == 0" + end).c_str()));
}

Token *CheckMemoryLeakClass::getcode(const Token *tok, std::list<const Token *> callstack, const char varname[], AllocType &alloctype, AllocType &dealloctype, bool classmember, bool &all, unsigned int sz)
{
    const char *varnames[2];
    varnames[0] = varname;
    varnames[1] = 0;
    std::string varnameStr = varname;

    Token *rethead = 0, *rettail = 0;
#define addtoken(_str)                  \
    {                                       \
        if (rettail)                        \
        {                                   \
            rettail->insertToken(_str);     \
            rettail = rettail->next();      \
        }                                   \
        else                                \
        {                                   \
            rethead = new Token;            \
            rettail = rethead;              \
            rettail->str(_str);             \
        }                                   \
                                            \
        rettail->linenr( tok->linenr() );   \
        rettail->fileIndex( tok->fileIndex() ); \
    }

    // The first token should be ";"
    addtoken(";");

    bool isloop = false;

    int indentlevel = 0;
    int parlevel = 0;
    for (; tok; tok = tok->next())
    {
        if (tok->str() == "{")
        {
            addtoken("{");
            ++indentlevel;
        }
        else if (tok->str() == "}")
        {
            addtoken("}");
            if (indentlevel <= 0)
                break;
            --indentlevel;
        }

        if (tok->str() == "(")
            ++parlevel;
        else if (tok->str() == ")")
            --parlevel;
        isloop &= (parlevel > 0);

        if (parlevel == 0 && tok->str() == ";")
            addtoken(";");

        if (Token::Match(tok->previous(), std::string("[(;{}] " + varnameStr + " =").c_str()))
        {
            AllocType alloc = GetAllocationType(tok->tokAt(2));
            bool realloc = false;

            if (sz > 1 &&
                Token::Match(tok->tokAt(2), "malloc ( %num% )") &&
                (std::atoi(tok->strAt(4)) % sz) != 0)
            {
                _errorLogger->mismatchSize(_tokenizer, tok->tokAt(4), tok->strAt(4));
            }

            if (alloc == No)
            {
                alloc = GetReallocationType(tok->tokAt(2));
                if (alloc != No)
                {
                    addtoken("realloc");
                    addtoken(";");
                    realloc = true;
                }
            }

            // If "--all" hasn't been given, don't check classes..
            if (alloc == New)
            {
                if (Token::Match(tok->tokAt(2), "new %type% [(;]"))
                {
                    if (isclass(tok->tokAt(3)))
                    {
                        if (_settings._showAll)
                        {

                            if (_settings.isAutoDealloc(tok->strAt(3)))
                            {
                                // This class has automatic deallocation
                                alloc = No;
                            }
                            else
                            {
                                // The checking will proceed.. but any error messages that are shown are shown thanks to "--all"
                                all = true;
                            }
                        }
                        else
                            alloc = No;
                    }
                }
            }

            if (alloc != No)
            {
                if (! realloc)
                    addtoken("alloc");

                if (alloctype != No && alloctype != alloc)
                    alloc = Many;

                if (alloc != Many && dealloctype != No && dealloctype != Many && dealloctype != alloc)
                {
                    callstack.push_back(tok);
                    _errorLogger->mismatchAllocDealloc(_tokenizer, callstack, varname);
                    callstack.pop_back();
                }

                alloctype = alloc;
            }

            else if (MatchFunctionsThatReturnArg(tok->previous(), std::string(varname)))
            {
                addtoken("use");
            }

            // assignment..
            else
            {
                // is the pointer in rhs?
                bool rhs = false;
                std::string pattern("[=+] " + std::string(varname));
                for (const Token *tok2 = tok; tok2; tok2 = tok2->next())
                {
                    if (tok2->str() == ";")
                        break;

                    if (Token::Match(tok2, pattern.c_str()))
                    {
                        rhs = true;
                        break;
                    }
                }

                addtoken((rhs ? "use" : "assign"));
            }
        }

        if (Token::Match(tok->previous(), "[;{})] %var%"))
        {
            AllocType dealloc = GetDeallocationType(tok, varnames);
            if (dealloc != No)
            {
                addtoken("dealloc");

                if (dealloctype != No && dealloctype != dealloc)
                    dealloc = Many;

                if (dealloc != Many && alloctype != No && alloctype != Many && alloctype != dealloc)
                {
                    callstack.push_back(tok);
                    _errorLogger->mismatchAllocDealloc(_tokenizer, callstack, varname);
                    callstack.pop_back();
                }
                dealloctype = dealloc;
                continue;
            }
        }

        // if else switch
        if (tok->str() == "if")
        {
            if (Token::simpleMatch(tok, std::string("if ( " + varnameStr + " )").c_str()) ||
                Token::simpleMatch(tok, std::string("if ( " + varnameStr + " != 0 )").c_str()) ||
                Token::simpleMatch(tok, std::string("if ( 0 != " + varnameStr + " )").c_str()))
            {
                addtoken("if(var)");

                // Make sure the "use" will not be added
                while (tok->str() != ")")
                    tok = tok->next();
            }
            else if (Token::simpleMatch(tok, "if (") && notvar(tok->tokAt(2), varnames, true))
            {
                addtoken("if(!var)");
            }
            else
            {
                // Check if the condition depends on var somehow..
                bool dep = false;
                int parlevel = 0;
                for (const Token *tok2 = tok; tok2; tok2 = tok2->next())
                {
                    if (tok2->str() == "(")
                        ++parlevel;
                    if (tok2->str() == ")")
                    {
                        --parlevel;
                        if (parlevel <= 0)
                            break;
                    }
                    if (Token::simpleMatch(tok2, std::string("fclose ( " + varnameStr + " )").c_str()))
                    {
                        addtoken("dealloc");
                        addtoken(";");
                        dep = true;
                        break;
                    }
                    if ((tok2->str() != ".") &&
                        Token::simpleMatch(tok2->next(), varnameStr.c_str()) &&
                        !Token::simpleMatch(tok2->next(), std::string(varnameStr + " .").c_str()))
                    {
                        dep = true;
                        break;
                    }
                }
                addtoken((dep ? "ifv" : "if"));
            }
        }

        if ((tok->str() == "else") || (tok->str() == "switch"))
        {
            addtoken(tok->aaaa());
        }

        if ((tok->str() == "case"))
        {
            addtoken("case");
            addtoken(";");
        }

        if ((tok->str() == "default"))
        {
            addtoken("default");
            addtoken(";");
        }

        // Loops..
        if ((tok->str() == "for") || (tok->str() == "while"))
        {
            addtoken("loop");
            isloop = true;
        }
        if ((tok->str() == "do"))
        {
            addtoken("do");
        }
        if (isloop && notvar(tok, varnames))
            addtoken("!var");

        // continue / break..
        if (tok->str() == "continue")
            addtoken("continue");
        if (tok->str() == "break")
            addtoken("break");

        // goto..
        if (tok->str() == "goto")
        {
            addtoken("goto");
        }

        // Return..
        if (tok->str() == "return")
        {
            addtoken("return");
            if (Token::simpleMatch(tok, std::string("return " + varnameStr).c_str()) ||
                Token::simpleMatch(tok, std::string("return & " + varnameStr).c_str()))
                addtoken("use");
            if (Token::simpleMatch(tok->next(), "("))
            {
                for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next())
                {
                    if (tok2->str() == "(" || tok2->str() == ")")
                        break;

                    if (tok2->str() == varname)
                    {
                        addtoken("use");
                        break;
                    }
                }
            }
            else if (Token::Match(tok, ("return strcpy|strncpy|memcpy ( " + varnameStr).c_str()))
            {
                addtoken("use");
                tok = tok->tokAt(2);
            }
        }

        // throw..
        if (Token::Match(tok, "try|throw|catch"))
            addtoken(tok->strAt(0));

        // Assignment..
        if (Token::Match(tok, std::string("[)=] " + varnameStr + " [+;)]").c_str()) ||
            Token::Match(tok, std::string(varnameStr + " +=|-=").c_str()) ||
            Token::Match(tok, std::string("+=|<< " + varnameStr + " ;").c_str()))
        {
            addtoken("use");
        }
        else if (Token::Match(tok->previous(), std::string("[;{}=(,+-*/] " + varnameStr + " [").c_str()))
        {
            addtoken("use_");
        }

        // Investigate function calls..
        if (Token::Match(tok, "%var% ("))
        {
            // Inside class function.. if the var is passed as a parameter then
            // just add a "::use"
            // The "::use" means that a member function was probably called but it wasn't analyzed further
            if (classmember)
            {
                int parlevel = 1;
                for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next())
                {
                    if (tok2->str() == "(")
                        ++parlevel;
                    else if (tok2->str() == ")")
                    {
                        --parlevel;
                        if (parlevel <= 0)
                            break;
                    }
                    if (tok2->str() == varnameStr)
                    {
                        addtoken("::use");
                        break;
                    }
                }
            }

            else
            {
                const char *str = call_func(tok, callstack, varnames, alloctype, dealloctype, all, sz);
                if (str)
                    addtoken(str);
            }
        }

        // Callback..
        bool matchFirst;
        if ((matchFirst = Token::Match(tok, "( %var%")) ||
            Token::Match(tok, "( * %var%"))
        {
            int tokIdx = matchFirst ? 2 : 3;

            while (Token::simpleMatch(tok->tokAt(tokIdx), ".") &&
                   Token::Match(tok->tokAt(tokIdx + 1), "%var%"))
                tokIdx += 2;

            if (Token::simpleMatch(tok->tokAt(tokIdx), ") ("))
            {
                for (const Token *tok2 = tok->tokAt(tokIdx + 2); tok2; tok2 = tok2->next())
                {
                    if (Token::Match(tok2, "[;{]"))
                        break;
                    else if (tok2->str() == varname)
                    {
                        addtoken("use");
                        break;
                    }
                }
            }
        }

        // Linux lists..
        if (Token::Match(tok, std::string("[=(,] & " + varnameStr + " [.[,)]").c_str()))
        {
            addtoken("&use");
        }
    }

    return rethead;
}

void CheckMemoryLeakClass::erase(Token *begin, const Token *end)
{
    Token::eraseTokens(begin, end);
}

void CheckMemoryLeakClass::simplifycode(Token *tok, bool &all)
{
    // Replace "throw" that is not in a try block with "return"
    int indentlevel = 0;
    int trylevel = -1;
    for (Token *tok2 = tok; tok2; tok2 = tok2->next())
    {
        if (tok2->str() == "{")
            ++indentlevel;
        else if (tok2->str() == "}")
        {
            --indentlevel;
            if (indentlevel <= trylevel)
                trylevel = -1;
        }
        else if (trylevel == -1 && tok2->str() == "try")
            trylevel = indentlevel;
        else if (trylevel == -1 && tok2->str() == "throw")
            tok2->str("return");
    }

    // reduce the code..
    bool done = false;
    while (! done)
    {
        done = true;

        for (Token *tok2 = tok; tok2; tok2 = tok2 ? tok2->next() : NULL)
        {
            // Delete extra ";"
            while (Token::Match(tok2, "[;{}] ;"))
            {
                erase(tok2, tok2->tokAt(2));
                done = false;
            }

            // Replace "{ }" with ";"
            if (Token::Match(tok2->next(), "{ }"))
            {
                tok2->next()->str(";");
                erase(tok2->next(), tok2->tokAt(3));
                done = false;
            }

            // Delete braces around a single instruction..
            if (Token::Match(tok2->next(), "{ %var% ; }"))
            {
                erase(tok2, tok2->tokAt(2));
                erase(tok2->next()->next(), tok2->tokAt(4));
                done = false;
            }
            if (Token::Match(tok2->next(), "{ %var% %var% ; }"))
            {
                erase(tok2, tok2->tokAt(2));
                erase(tok2->next()->next()->next(), tok2->tokAt(5));
                done = false;
            }


            if (Token::simpleMatch(tok2->next(), "if"))
            {
                // Delete empty if that is not followed by an else
                if (Token::Match(tok2->next(), "if ; !!else"))
                {
                    erase(tok2, tok2->tokAt(2));
                    done = false;
                }

                // Delete "if ; else ;"
                else if (Token::Match(tok2->next(), "if ; else ;"))
                {
                    erase(tok2, tok2->tokAt(4));
                    done = false;
                }

                // Two "if alloc ;" after one another.. perhaps only one of them can be executed each time
                else if (!_settings._showAll && Token::Match(tok2, "[;{}] if alloc ; if alloc ;"))
                {
                    erase(tok2, tok2->tokAt(4));
                    done = false;
                }

                // TODO Make this more generic. Delete "if ; else use ; use"
                else if (Token::Match(tok2, "; if ; else assign|use ; assign|use") ||
                         Token::Match(tok2, "; if assign|use ; else ; assign|use"))
                {
                    erase(tok2, tok2->tokAt(4));
                    done = false;
                }


                // Reduce "if assign|dealloc|use ;" that is not followed by an else..
                // If "--all" has been given these are deleted
                // Otherwise, only the "if" will be deleted
                else if (Token::Match(tok2, "[;{}] if assign|dealloc|use ; !!else"))
                {
                    if (_settings._showAll)
                    {
                        erase(tok2, tok2->tokAt(3));
                        all = true;
                    }
                    else
                    {
                        erase(tok2, tok2->tokAt(2));
                    }
                    done = false;
                }

                // Reduce "if if" => "if"
                else if (Token::Match(tok2, "if if"))
                {
                    erase(tok2, tok2->tokAt(2));
                    done = false;
                }

                // Reduce "if return ; alloc ;" => "alloc ;"
                else if (Token::Match(tok2, "[;{}] if return ; alloc ;"))
                {
                    erase(tok2, tok2->tokAt(4));
                    done = false;
                }

                // "[;{}] if alloc ; else return ;" => "[;{}] alloc ;"
                else if (Token::Match(tok2, "[;{}] if alloc ; else return ;"))
                {
                    erase(tok2, tok2->tokAt(2));        // Remove "if"
                    erase(tok2->next(), tok2->tokAt(5));  // Remove "; else return"
                    done = false;
                }

                // Reduce "if ; else %var% ;" => "if %var% ;"
                else if (Token::Match(tok2->next(), "if ; else %var% ;"))
                {
                    erase(tok2->next(), tok2->tokAt(4));
                    done = false;
                }

                // Reduce "if ; else return use ;" => "if return use ;"
                else if (Token::Match(tok2->next(), "if ; else return use ;"))
                {
                    erase(tok2->next(), tok2->tokAt(4));
                    done = false;
                }

                // Reduce "if return ; if return ;" => "if return ;"
                else if (Token::Match(tok2->next(), "if return ; if return ;"))
                {
                    erase(tok2, tok2->tokAt(4));
                    done = false;
                }

                // Delete first if in .. "if { dealloc|assign|use ; return ; } if return ;"
                else if (Token::Match(tok2, "[;{}] if { dealloc|assign|use ; return ; } if return ;"))
                {
                    erase(tok2, tok2->tokAt(8));
                    done = false;
                }

                // Remove "if { dealloc ; callfunc ; } !!else"
                else if (Token::Match(tok2->next(), "if { dealloc|assign|use ; callfunc ; } !!else"))
                {
                    erase(tok2, tok2->tokAt(8));
                    done = false;
                }

                // Reducing if..
                else if (_settings._showAll)
                {
                    if (Token::Match(tok2, "[;{}] if { assign|dealloc|use ; return ; } !!else"))
                    {
                        all = true;
                        erase(tok2, tok2->tokAt(8));
                        done = false;
                    }
                }

                continue;
            }

            // Reduce "if(var) dealloc ;" and "if(var) use ;" that is not followed by an else..
            if (Token::Match(tok2, "[;{}] if(var) assign|dealloc|use ; !!else"))
            {
                erase(tok2, tok2->tokAt(2));
                done = false;
            }

            // Reduce "; if(!var) alloc ; !!else" => "; dealloc ; alloc ;"
            if (Token::Match(tok2, "; if(!var) alloc ; !!else"))
            {
                // Remove the "if(!var)"
                erase(tok2, tok2->tokAt(2));

                // Insert "dealloc ;" before the "alloc ;"
                tok2->insertToken(";");
                tok2->insertToken("dealloc");

                done = false;
            }

            // Remove "catch ;"
            if (Token::simpleMatch(tok2->next(), "catch ;"))
            {
                erase(tok2, tok2->tokAt(3));
                done = false;
            }

            // Reduce "if* ;" that is not followed by an else..
            if (Token::Match(tok2->next(), "if(var)|if(!var)|ifv ; !!else"))
            {
                erase(tok2, tok2->tokAt(2));
                done = false;
            }

            // Reduce "else ;" => ";"
            if (Token::Match(tok2->next(), "else ;"))
            {
                erase(tok2, tok2->tokAt(2));
                done = false;
            }

            // Delete if block: "alloc; if return use ;"
            if (Token::Match(tok2, "alloc ; if return use ; !!else"))
            {
                erase(tok2, tok2->tokAt(5));
                done = false;
            }


            // Replace "dealloc use ;" with "dealloc ;"
            if (Token::Match(tok2, "dealloc use ;"))
            {
                erase(tok2, tok2->tokAt(2));
                done = false;
            }

            // Remove the "if break|continue ;" that follows "dealloc ; alloc ;"
            if (! _settings._showAll && Token::Match(tok2, "dealloc ; alloc ; if break|continue ;"))
            {
                tok2 = tok2->next()->next()->next();
                erase(tok2, tok2->tokAt(3));
                done = false;
            }

            // Reduce "do { alloc ; } " => "alloc ;"
            // TODO: If the loop can be executed twice reduce to "loop alloc ;" instead
            if (Token::Match(tok2->next(), "do { alloc ; }"))
            {
                erase(tok2, tok2->tokAt(3));
                erase(tok2->next()->next(), tok2->tokAt(4));
                done = false;
            }

            // Reduce "loop if break ; => ";"
            if (Token::Match(tok2->next(), "loop if break|continue ; !!else"))
            {
                erase(tok2, tok2->tokAt(4));
                done = false;
            }

            // Reduce "loop { assign|dealloc|use ; alloc ; if break ; }" to "assign|dealloc|use ; alloc ;"
            if (Token::Match(tok2->next(), "loop { assign|dealloc|use ; alloc ; if break|continue ; }"))
            {
                // erase "loop {"
                erase(tok2, tok2->tokAt(3));
                // erase "if break|continue ; }"
                tok2 = tok2->next()->next()->next()->next();
                erase(tok2, tok2->tokAt(5));
                done = false;
            }

            // Replace "loop { X ; break ; }" with "X ;"
            if (Token::Match(tok2->next(), "loop { %var% ; break ; }"))
            {
                erase(tok2, tok2->tokAt(3));
                erase(tok2->next()->next(), tok2->tokAt(6));
                done = false;
            }

            // Replace "loop ;" with ";"
            if (Token::Match(tok2->next(), "loop ;"))
            {
                erase(tok2, tok2->tokAt(2));
                done = false;
            }

            // Replace "loop !var ;" with ";"
            if (Token::Match(tok2->next(), "loop !var ;"))
            {
                erase(tok2, tok2->tokAt(4));
                done = false;
            }

            // Replace "loop !var alloc ;" with " alloc ;"
            if (Token::Match(tok2->next(), "loop !var alloc ;"))
            {
                erase(tok2, tok2->tokAt(3));
                done = false;
            }

            // Replace "loop if return ;" with "if return ;"
            if (Token::Match(tok2->next(), "loop if return"))
            {
                erase(tok2, tok2->tokAt(2));
                done = false;
            }

            // Delete if block in "alloc ; if(!var) return ;"
            if (Token::Match(tok2, "alloc ; if(!var) return ;"))
            {
                erase(tok2, tok2->tokAt(4));
                done = false;
            }

            // Delete if block: "alloc; if return use ;"
            if (Token::Match(tok2, "alloc ; if return use ; !!else"))
            {
                erase(tok2, tok2->tokAt(5));
                done = false;
            }

            // Reduce "[;{}] return ; %var%" => "[;{}] return ;"
            if (Token::Match(tok2, "[;{}] return ; %var%"))
            {
                erase(tok2->next()->next(), tok2->tokAt(4));
                done = false;
            }

            // Reduce "[;{}] return use ; %var%" => "[;{}] return use ;"
            if (Token::Match(tok2, "[;{}] return use ; %var%"))
            {
                erase(tok2->next()->next()->next(), tok2->tokAt(5));
                done = false;
            }

            // Reduce "if(var) return use ;" => "return use ;"
            if (Token::Match(tok2->next(), "if(var) return use ; !!else"))
            {
                erase(tok2, tok2->tokAt(2));
                done = false;
            }

            // Reduce "if(var) assign|dealloc|use ;" => "assign|dealloc|use ;"
            if (Token::Match(tok2->next(), "if(var) assign|dealloc|use ; !!else"))
            {
                erase(tok2, tok2->tokAt(2));
                done = false;
            }

            // malloc - realloc => alloc ; dealloc ; alloc ;
            // Reduce "[;{}] alloc ; dealloc ; alloc ;" => "[;{}] alloc ;"
            if (Token::Match(tok2, "[;{}] alloc ; dealloc ; alloc ;"))
            {
                erase(tok2->next(), tok2->tokAt(6));
                done = false;
            }

            // Reduce "if* alloc ; dealloc ;" => ";"
            if (Token::Match(tok2->tokAt(2), "alloc ; dealloc ;") &&
                tok2->next()->str().find("if") == 0)
            {
                erase(tok2, tok2->tokAt(5));
                done = false;
            }

            // Delete second use in "use ; use ;"
            while (Token::Match(tok2, "[;{}] use ; use ;"))
            {
                erase(tok2, tok2->tokAt(3));
                done = false;
            }

            // Delete first part in "use ; dealloc ;"
            if (Token::Match(tok2, "[;{}] use ; dealloc ;"))
            {
                erase(tok2, tok2->tokAt(3));
                done = false;
            }

            // Delete first part in "use ; return use ;"
            if (Token::Match(tok2, "[;{}] use ; return use ;"))
            {
                erase(tok2, tok2->tokAt(2));
                done = false;
            }

            // Delete second case in "case ; case ;"
            while (Token::Match(tok2, "case ; case ;"))
            {
                erase(tok2, tok2->tokAt(3));
                done = false;
            }

            // Replace switch with if (if not complicated)
            if (Token::Match(tok2, "switch {"))
            {
                // Right now, I just handle if there are a few case and perhaps a default.
                bool valid = false;
                bool incase = false;
                for (const Token * _tok = tok2->tokAt(2); _tok; _tok = _tok->next())
                {
                    if (_tok->str() == "{")
                        break;

                    else if (_tok->str() == "}")
                    {
                        valid = true;
                        break;
                    }

                    else if (strncmp(_tok->aaaa(), "if", 2) == 0)
                        break;

                    else if (_tok->str() == "switch")
                        break;

                    else if (_tok->str() == "loop")
                        break;

                    else if (incase && Token::Match(_tok, "case"))
                        break;

                    incase |= Token::Match(_tok, "case");
                    incase &= !Token::Match(_tok, "break");
                }

                if (!incase && valid)
                {
                    done = false;
                    tok2->str(";");
                    erase(tok2, tok2->tokAt(2));
                    tok2 = tok2->next();
                    bool first = true;
                    while (Token::Match(tok2, "case") || Token::Match(tok2, "default"))
                    {
                        bool def = Token::Match(tok2, "default");
                        tok2->str(first ? "if" : "}");
                        if (first)
                        {
                            first = false;
                            tok2->insertToken("{");
                        }
                        else
                        {
                            // Insert "else [if] {
                            tok2->insertToken("{");
                            if (! def)
                                tok2->insertToken("if");
                            tok2->insertToken("else");
                            tok2 = tok2->next();
                        }
                        while (tok2 && tok2->str() != "}" && ! Token::Match(tok2, "break ;"))
                            tok2 = tok2->next();
                        if (Token::Match(tok2, "break ;"))
                        {
                            tok2->str(";");
                            tok2 = tok2->next()->next();
                        }
                    }
                }
            }
        }
    }
}





// Check for memory leaks for a function variable.
void CheckMemoryLeakClass::CheckMemoryLeak_CheckScope(const Token *Tok1, const char varname[], bool classmember, unsigned int sz)
{
    std::list<const Token *> callstack;

    AllocType alloctype = No;
    AllocType dealloctype = No;

    bool all = false;

    const Token *result;

    Token *tok = getcode(Tok1, callstack, varname, alloctype, dealloctype, classmember, all, sz);
    //tok->printOut( "getcode result" );

    // Simplify the code and check if freed memory is used..
    for (Token *tok2 = tok; tok2; tok2 = tok2->next())
    {
        while (Token::Match(tok2, "[;{}] ;"))
            erase(tok2, tok2->tokAt(2));
    }
    if ((result = Token::findmatch(tok, "dealloc [;{}] use|use_ ;")) != NULL)
    {
        _errorLogger->deallocuse(_tokenizer, result->tokAt(2), varname);
    }

    // Replace "&use" with "use". Replace "use_" with ";"
    for (Token *tok2 = tok; tok2; tok2 = tok2->next())
    {
        if (tok2->str() == "&use")
            tok2->str("use");
        else if (tok2->str() == "use_")
            tok2->str(";");
        else if (tok2->str() == "::use")    // Some kind of member function usage. Not analyzed very well.
            tok2->str("use");
        else if (tok2->str() == "recursive")
            tok2->str("use");
        else if (tok2->str() == "dealloc_")
            tok2->str("dealloc");
        else if (tok2->str() == "realloc")
        {
            tok2->str("dealloc");
            tok2->insertToken("alloc");
            tok2->insertToken(";");
        }
    }

    simplifycode(tok, all);
    //tok->printOut("simplifycode result");

    // If the variable is not allocated at all => no memory leak
    if (Token::findmatch(tok, "alloc") == 0)
    {
        Tokenizer::deleteTokens(tok);
        return;
    }

    // TODO : handle "goto"
    if (Token::findmatch(tok, "goto"))
    {
        Tokenizer::deleteTokens(tok);
        return;
    }

    if ((result = Token::findmatch(tok, "loop alloc ;")) != NULL)
    {
        MemoryLeak(result, varname, alloctype, all);
    }

    else if ((result = Token::findmatch(tok, "alloc ; if break|continue|return ;")) != NULL
             && Token::findmatch(tok, "dealloc ; alloc ; if continue ;") == NULL)
    {
        MemoryLeak(result->tokAt(3), varname, alloctype, all);
    }

    else if (_settings._showAll && (result = Token::findmatch(tok, "alloc ; ifv break|continue|return ;")) != NULL)
    {
        MemoryLeak(result->tokAt(3), varname, alloctype, all);
    }

    else if ((result = Token::findmatch(tok, "alloc ; alloc|assign|return ;")) != NULL)
    {
        MemoryLeak(result->tokAt(2), varname, alloctype, all);
    }

    else if ((result = Token::findmatch(tok, "dealloc ; dealloc ;")) != NULL)
    {
        _errorLogger->deallocDealloc(_tokenizer, result->tokAt(2), varname);
    }

    else if (! Token::findmatch(tok, "dealloc") &&
             ! Token::findmatch(tok, "use") &&
             ! Token::findmatch(tok, "return use ;"))
    {
        const Token *last = tok;
        while (last->next())
            last = last->next();
        MemoryLeak(last, varname, alloctype, all);
    }

    // detect cases that "simplifycode" don't handle well..
    else if (_settings._debug)
    {
        Token *first = tok;
        while (first && first->str() == ";")
            first = first->next();

        bool noerr = false;
        noerr |= Token::Match(first, "alloc ; }");
        noerr |= Token::Match(first, "alloc ; dealloc ; }");
        noerr |= Token::Match(first, "alloc ; return use ; }");
        noerr |= Token::Match(first, "alloc ; use ; }");
        noerr |= Token::Match(first, "alloc ; use ; return ; }");
        noerr |= Token::Match(first, "if alloc ; dealloc ; }");
        noerr |= Token::Match(first, "if alloc ; return use ; }");
        noerr |= Token::Match(first, "if alloc ; use ; }");
        noerr |= Token::Match(first, "alloc ; ifv return ; dealloc ; }");
        noerr |= Token::Match(first, "alloc ; if return ; dealloc; }");

        // Unhandled case..
        if (! noerr)
        {
            std::cout << "Token listing..\n  ";
            for (const Token *tok2 = tok; tok2; tok2 = tok2->next())
                std::cout << " " << tok2->str();
            std::cout << "\n";
        }
    }

    Tokenizer::deleteTokens(tok);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// Checks for memory leaks inside function..
//---------------------------------------------------------------------------

void CheckMemoryLeakClass::CheckMemoryLeak_InFunction()
{
    bool classmember = false;
    bool infunc = false;
    int indentlevel = 0;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->str() == "{")
            ++indentlevel;

        else if (tok->str() == "}")
            --indentlevel;

        // In function..
        if (indentlevel == 0)
        {
            if (Token::Match(tok, ") {"))
                infunc = true;

            else if (tok->str() == "::")
                classmember = true;

            else if (Token::Match(tok, "[;}]"))
                infunc = classmember = false;
        }

        // Declare a local variable => Check
        if (indentlevel > 0 && infunc)
        {
            unsigned int sz = _tokenizer->SizeOfType(tok->strAt(1));
            if (sz < 1)
                sz = 1;

            if (Token::Match(tok, "[{};] %type% * %var% [;=]"))
                CheckMemoryLeak_CheckScope(tok->next(), tok->strAt(3), classmember, sz);

            else if (Token::Match(tok, "[{};] %type% %type% * %var% [;=]"))
                CheckMemoryLeak_CheckScope(tok->next(), tok->strAt(4), classmember, sz);
        }
    }
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Checks for memory leaks in classes..
//---------------------------------------------------------------------------



void CheckMemoryLeakClass::CheckMemoryLeak_ClassMembers()
{
    int indentlevel = 0;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->str() == "{")
            ++indentlevel;

        else if (tok->str() == "}")
            --indentlevel;

        else if (indentlevel == 0 && Token::Match(tok, "class %var% [{:]"))
        {
            std::vector<const char *> classname;
            classname.push_back(tok->strAt(1));
            CheckMemoryLeak_ClassMembers_ParseClass(tok, classname);
        }
    }
}


void CheckMemoryLeakClass::CheckMemoryLeak_ClassMembers_ParseClass(const Token *tok1, std::vector<const char *> &classname)
{
    // Go into class.
    while (tok1 && tok1->str() != "{")
        tok1 = tok1->next();
    if (tok1)
        tok1 = tok1->next();

    int indentlevel = 0;
    for (const Token *tok = tok1; tok; tok = tok->next())
    {
        if (tok->str() == "{")
            ++indentlevel;

        else if (tok->str() == "}")
        {
            --indentlevel;
            if (indentlevel < 0)
                return;
        }

        // Only parse this particular class.. not subclasses
        if (indentlevel > 0)
            continue;

        // Declaring subclass.. recursive checking
        if (Token::Match(tok, "class %var% [{:]"))
        {
            classname.push_back(tok->strAt(1));
            CheckMemoryLeak_ClassMembers_ParseClass(tok, classname);
            classname.pop_back();
        }

        // Declaring member variable.. check allocations and deallocations
        if (Token::Match(tok->next(), "%type% * %var% ;"))
        {
            // No false positives for auto deallocated classes..
            if (_settings.isAutoDealloc(tok->strAt(1)))
                continue;

            if (tok->isName() || Token::Match(tok, "[;}]"))
            {
                if (_settings._showAll || !isclass(tok->tokAt(1)))
                    CheckMemoryLeak_ClassMembers_Variable(classname.back(), tok->tokAt(3));
            }
        }
    }
}

void CheckMemoryLeakClass::CheckMemoryLeak_ClassMembers_Variable(const char classname[], const Token *tokVarname)
{
    const char *varname = tokVarname->strAt(0);

    // Check if member variable has been allocated and deallocated..
    AllocType Alloc = No;
    AllocType Dealloc = No;

    // Loop through all tokens. Inspect member functions
    int indent_ = 0;
    const Token *functionToken = Tokenizer::FindClassFunction(_tokenizer->tokens(), classname, "~| %var%", indent_);
    while (functionToken)
    {
        int indent = 0;
        for (const Token *tok = functionToken; tok; tok = tok->next())
        {
            if (tok->str() == "{")
                ++indent;
            else if (tok->str() == "}")
            {
                --indent;
                if (indent <= 0)
                    break;
            }
            else if (indent > 0)
            {
                // Allocate..
                if (Token::Match(tok, (std::string(varname) + " =").c_str()))
                {
                    AllocType alloc = GetAllocationType(tok->tokAt(2));
                    if (alloc != No)
                    {
                        if (Alloc != No && Alloc != alloc)
                            alloc = Many;

                        std::list<const Token *> callstack;
                        if (alloc != Many && Dealloc != No && Dealloc != Many && Dealloc != alloc)
                        {
                            callstack.push_back(tok);
                            _errorLogger->mismatchAllocDealloc(_tokenizer, callstack, (std::string(classname) + "::" + varname).c_str());
                            callstack.pop_back();
                        }

                        Alloc = alloc;
                    }
                }

                // Deallocate..
                const char *varnames[3] = { "var", 0, 0 };
                varnames[0] = varname;
                AllocType dealloc = GetDeallocationType(tok, varnames);
                if (dealloc == No)
                {
                    varnames[0] = "this";
                    varnames[1] = varname;
                    dealloc = GetDeallocationType(tok, varnames);
                }
                if (dealloc != No)
                {
                    if (Dealloc != No && Dealloc != dealloc)
                        dealloc = Many;

                    std::list<const Token *> callstack;
                    if (dealloc != Many && Alloc != No &&  Alloc != Many && Alloc != dealloc)
                    {
                        callstack.push_back(tok);
                        _errorLogger->mismatchAllocDealloc(_tokenizer, callstack, (std::string(classname) + "::" + varname).c_str());
                        callstack.pop_back();
                    }

                    Dealloc = dealloc;
                }

            }
        }

        functionToken = Tokenizer::FindClassFunction(functionToken->next(), classname, "~| %var%", indent_);
    }

    if (Alloc != No && Dealloc == No)
    {
        MemoryLeak(tokVarname, (std::string(classname) + "::" + varname).c_str(), Alloc, true);
    }
}




//---------------------------------------------------------------------------
// Checks for memory leaks..
//---------------------------------------------------------------------------

void CheckMemoryLeakClass::CheckMemoryLeak()
{
    _listAllocFunc.clear();

    // Check for memory leaks inside functions..
    CheckMemoryLeak_InFunction();

    // Check that all class members are deallocated..
    if (_settings._showAll)
        CheckMemoryLeak_ClassMembers();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Non-recursive function analysis
//---------------------------------------------------------------------------

Token * CheckMemoryLeakClass::functionParameterCode(const Token *ftok, int parameter)
{
    int param = 1;  // First parameter has index 1

    // Extract the code for specified parameter...
    for (; ftok; ftok = ftok->next())
    {
        if (ftok->str() == ")")
            break;

        if (ftok->str() == ",")
        {
            ++param;
            if (param > parameter)
                break;
        }

        if (param != parameter)
            continue;

        if (! Token::Match(ftok, "* %var% [,)]"))
            continue;

        // Extract and return the code for this parameter..
        const char *parname = ftok->strAt(1);

        // Goto function implementation..
        while (ftok && ftok->str() != "{")
            ftok = ftok->next();
        ftok = ftok ? ftok->next() : NULL;

        // Return the code..
        AllocType alloc = No, dealloc = No;
        bool all = false;
        std::list<const Token *> callstack;
        Token *code = getcode(ftok, callstack, parname, alloc, dealloc, false, all, 1);
        simplifycode(code, all);
        return code;
    }

    return NULL;
}

