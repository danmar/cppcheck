#!/usr/bin/python

import re
import glob

def insertMatchStr(matchStrs, look_for):
    prefix = 'matchStr'

    # Add entry if needed
    if look_for not in matchStrs:
        pos = len(matchStrs) + 1
        matchStrs[look_for] = pos

    return prefix + str(matchStrs[look_for])

def compileCmd(tok, matchStrs):
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
        return '(tok->str()==' + insertMatchStr(matchStrs, '|') + ')/* | */'
    elif tok == '%oror%':
        return '(tok->str()==' + insertMatchStr(matchStrs, '||') + ')/* || */'
    elif tok == '%str%':
        return '(tok->type()==Token::eString)'
    elif tok == '%type%':
        return '(tok->isName() && tok->varId()==0U && tok->str() != ' + insertMatchStr(matchStrs, 'delete') + '/* delete */)'
    elif tok == '%var%':
        return 'tok->isName()'
    elif tok == '%varid%':
        return '(tok->isName() && tok->varId()==varid)'
    elif (len(tok)>2) and (tok[0]=="%"):
        print ("unhandled:" + tok)

    return '(tok->str()==' + insertMatchStr(matchStrs, tok) + ')/* ' + tok + ' */'

def compilePattern(matchStrs, pattern, nr, varid):
    arg2 = ''
    if varid:
        arg2 = ', const unsigned int varid'
    ret = '// ' + pattern + '\n'
    ret += 'static bool match' + str(nr) + '(const Token *tok'+arg2+') {\n'
    if varid:
        # if varid is provided, check that it's non-zero
        ret += '    if (varid==0U)\n'
        ret += '        throw InternalError(tok, "Internal error. Token::Match called with varid 0. Please report this to Cppcheck developers");\n'

    tokens = pattern.split(' ')
    gotoNextToken = ''
    for tok in tokens:
        if tok == '':
            continue
        ret += gotoNextToken
        gotoNextToken = '    tok = tok->next();\n'

        # [abc]
        if (len(tok) > 2) and (tok[0] == '[') and (tok[-1] == ']'):
            ret += '    if (!tok || tok->str().size()!=1U || !strchr("'+tok[1:-1]+'", tok->str()[0]))\n'
            ret += '        return false;\n'

        # a|b|c
        elif tok.find('|') > 0:
            tokens2 = tok.split('|')
            logicalOp = None
            neg = None
            if "" in tokens2:
                ret += '    if (tok && ('
                logicalOp = ' || '
                neg = ''
            else:
                ret += '    if (!tok || !('
                logicalOp = ' || '
                neg = ''
            first = True
            for tok2 in tokens2:
                if tok2 == '':
                    continue
                if not first:
                    ret += logicalOp
                first = False
                ret += neg + compileCmd(tok2, matchStrs)

            if "" in tokens2:
                ret += '))\n'
                ret += '        tok = tok->next();\n'
                gotoNextToken = ''
            else:
                ret += '))\n'
                ret += '        return false;\n'

        # !!a
        elif tok[0:2]=="!!":
            ret += '    if (tok && tok->str() == ' + insertMatchStr(matchStrs, tok[2:]) + ')/* ' + tok[2:] + ' */\n'
            ret += '        return false;\n'
            gotoNextToken = '    tok = tok ? tok->next() : NULL;\n'

        else:
            ret += '    if (!tok || !' + compileCmd(tok, matchStrs) + ')\n'
            ret += '        return false;\n'
    ret += '    return true;\n}\n'

    return ret

def parseMatch(line, pos1):
    parlevel = 0
    args = []
    argstart = 0
    pos = pos1
    inString = False
    while pos < len(line):
        if inString:
            if line[pos] == '\\':
                pos += 1
            elif line[pos] == '"':
                inString = False
        elif line[pos] == '"':
            inString = True
        elif line[pos] == '(':
            parlevel += 1
            if parlevel == 1:
                argstart = pos + 1
        elif line[pos] == ')':
            parlevel -= 1
            if parlevel == 0:
                ret = []
                ret.append(line[pos1:pos+1])
                for arg in args:
                    ret.append(arg)
                ret.append(line[argstart:pos])
                return ret
        elif line[pos] == ',' and parlevel == 1:
            args.append(line[argstart:pos])
            argstart = pos + 1
        pos += 1

    return None

def parseStringComparison(line, pos1):
    startPos = 0
    endPos = 0
    pos = pos1
    inString = False
    while pos < len(line):
        if inString:
            if line[pos] == '\\':
                pos += 1
            elif line[pos] == '"':
                inString = False
                endPos = pos+1
                return (startPos, endPos)
        elif line[pos] == '"':
            startPos = pos
            inString = True
        pos += 1

    return None

def replaceTokenMatch(matchFunctions, matchStrs, line):
    while True:
        pos1 = line.find('Token::Match(')
        if pos1 == -1:
            pos1 = line.find('Token::simpleMatch(')
        if pos1 == -1:
            break

        res = parseMatch(line, pos1)
        if res == None:
            break
        else:
            assert(len(res)==3 or len(res)==4)  # assert that Token::Match has either 2 or 3 arguments

            g0 = res[0]
            arg1 = res[1]
            arg2 = res[2]
            arg3 = None
            if len(res) == 4:
                arg3 = res[3]

            res = re.match(r'\s*"([^"]*)"\s*$', arg2)
            if res == None:
                break  # Non-const pattern - bailout
            else:
                arg2 = res.group(1)
                a3 = ''
                if arg3:
                    a3 = ',' + arg3
                patternNumber = len(matchFunctions) + 1
                line = line[:pos1]+'match'+str(patternNumber)+'('+arg1+a3+')'+line[pos1+len(g0):]
                matchFunctions.append(compilePattern(matchStrs, arg2, patternNumber, arg3))

    return line

def replaceCStrings(matchStrs, line):
    while True:
        match = re.search('str\(\) (==|!=) "', line)
        if not match:
            match = re.search('strAt\(.+?\) (==|!=) "', line)
        if not match:
            break

        res = parseStringComparison(line, match.start())
        if res == None:
            break

        startPos = res[0]
        endPos = res[1]
        text = line[startPos+1:endPos-1]
        line = line[:startPos] + insertMatchStr(matchStrs, text) + line[endPos:]

    return line

def convertFile(srcname, destname):
    fin = open(srcname, "rt")
    srclines = fin.readlines()
    fin.close()

    header = '#include "token.h"\n'
    header += '#include "errorlogger.h"\n'
    header += '#include <string>\n'
    header += '#include <cstring>\n'
    matchFunctions = []
    code = ''

    matchStrs = {}
    for line in srclines:
        # Compile Token::Match and Token::simpleMatch
        line = replaceTokenMatch(matchFunctions, matchStrs, line)

        # Cache plain C-strings in C++ strings
        line = replaceCStrings(matchStrs, line)

        code += line

    # Compute string list
    stringList = ''
    for match in sorted(matchStrs, key=matchStrs.get):
        stringList += 'static const std::string matchStr' + str(matchStrs[match]) + '("' + match + '");\n'

    # Compute matchFunctions
    strFunctions = ''
    for function in matchFunctions:
        strFunctions += function;

    fout = open(destname, 'wt')
    fout.write(header+stringList+strFunctions+code)
    fout.close()

# selftests..
def assertEquals(actual,expected):
    if actual!=expected:
        print ('Assertion failed:')
        print (actual)
        print (expected)
        assert actual == expected
assertEquals(parseMatch('  Token::Match(tok, ";") ',2), ['Token::Match(tok, ";")','tok',' ";"'])
assertEquals(parseMatch('  Token::Match(tok,', 2), None) # multiline Token::Match is not supported yet
assertEquals(parseMatch('  Token::Match(Token::findsimplematch(tok,")"), ";")', 2), ['Token::Match(Token::findsimplematch(tok,")"), ";")', 'Token::findsimplematch(tok,")")', ' ";"']) # inner function call

# convert all lib/*.cpp files
for f in glob.glob('lib/*.cpp'):
    print (f + ' => build/' + f[4:])
    convertFile(f, 'build/'+f[4:])
