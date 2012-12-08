#!/usr/bin/python

import re
import glob

def compileCmd(tok):
    if tok == '%any%':
        return 'true'
    elif tok == '%bool%':
        return 'tok->isBoolean()'
    elif tok == '%char%':
        return '(tok->type()==Token::eChar)'
    elif tok == '%comp%':
        return 'tok->isComparisonOp()'
    elif tok == '%num%':
        return 'tok->isNumber()'
    elif tok == '%op%':
        return 'tok->isOp()'
    elif tok == '%or%':
        return '(tok->str()=="|")'
    elif tok == '%oror%':
        return '(tok->str()=="||")'
    elif tok == '%num%':
        return 'tok->isNumber()'
    elif tok == '%str%':
        return '(tok->type()==Token::eString)'
    elif tok == '%type%':
        return '(tok->isName() && tok->varId()==0U && tok->str() != "delete")'
    elif tok == '%var%':
        return 'tok->isName()'
    elif (len(tok)>2) and (tok[0]=="%"):
        print "unhandled:" + tok
    return '(tok->str()=="'+tok+'")'

def compilePattern(pattern, nr):
    ret = '// ' + pattern + '\n'
    ret = ret + 'static bool match' + str(nr) + '(const Token *tok) {\n'
    tokens = pattern.split(' ')
    gotoNextToken = ''
    for tok in tokens:
        if tok == '':
            continue
        ret = ret + gotoNextToken
        gotoNextToken = '    tok = tok->next();\n'

        # [abc]
        if (len(tok) > 2) and (tok[0] == '[') and (tok[-1] == ']'):
            ret = ret + '    if (!tok || tok->str().size()!=1U || !strchr("'+tok[1:-1]+'", tok->str()[0]))\n'
            ret = ret + '        return false;\n'

        # a|b|c
        elif tok.find('|') > 0:
            tokens2 = tok.split('|')
            logicalOp = None
            neg = None
            if "" in tokens2:
                ret = ret + '    if (tok && ('
                logicalOp = ' || '
                neg = ''
            else:
                ret = ret + '    if (!tok || !('
                logicalOp = ' || '
                neg = ''
            first = True
            for tok2 in tokens2:
                if tok2 == '':
                    continue
                if not first:
                    ret = ret + logicalOp
                first = False
                ret = ret + neg + compileCmd(tok2)

            if "" in tokens2:
                ret = ret + '))\n'
                ret = ret + '        tok = tok->next();\n'
                gotoNextToken = ''
            else:
                ret = ret + '))\n'
                ret = ret + '        return false;\n'

        # !!a
        elif tok[0:2]=="!!":
            ret = ret + '    if (tok && tok->str() == "' + tok[2:] + '")\n'
            ret = ret + '        return false;\n'
            gotoNextToken = '    tok = tok ? tok->next() : NULL;\n'

        else:
            ret = ret + '    if (!tok || !' + compileCmd(tok) + ')\n'
            ret = ret + '        return false;\n'
    ret = ret + '    return true;\n}\n'
    return ret

def findMatchPattern(line):
    res = re.search(r'Token::s?i?m?p?l?e?Match[(]([^(,]+),\s*"([^"]+)"[)]', line)
    return res

def convertFile(srcname, destname):
    fin = open(srcname, "rt")
    srclines = fin.readlines()
    fin.close()

    matchfunctions = ''
    matchfunctions = matchfunctions + '#include "token.h"\n'
    matchfunctions = matchfunctions + '#include <string>\n'
    matchfunctions = matchfunctions + '#include <cstring>\n'
    code = ''

    patternNumber = 1
    for line in srclines:
        res = findMatchPattern(line)
        if res == None: # or patternNumber > 68:
            code = code + line
        else:
            g0 = res.group(0)
            pos1 = line.find(g0)
            code = code + line[:pos1]+'match'+str(patternNumber)+'('+res.group(1)+')'+line[pos1+len(g0):]
            matchfunctions = matchfunctions + compilePattern(res.group(2), patternNumber)
            patternNumber = patternNumber + 1

    fout = open(destname, 'wt')
    fout.write(matchfunctions+code)
    fout.close()

# selftests..
assert(None != findMatchPattern(' Token::Match(tok, ";") '))
assert(None != findMatchPattern(' Token::simpleMatch(tok, ";") '))
assert(None == findMatchPattern(' Token::Match(tok->next(), ";") ')) # function calls are not handled

# convert all lib/*.cpp files
for f in glob.glob('lib/*.cpp'):
    print f + ' => build/' + f[4:]
    convertFile(f, 'build/'+f[4:])

