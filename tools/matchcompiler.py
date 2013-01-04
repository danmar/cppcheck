#!/usr/bin/python

import re
import glob

class MatchCompiler:
    def __init__(self):
        self._reset()
        self._selftests()

    def _reset(self):
        self._matchFunctions = []
        self._matchStrs = {}

    def _insertMatchStr(self, look_for):
        prefix = 'matchStr'

        # Add entry if needed
        if look_for not in self._matchStrs:
            pos = len(self._matchStrs) + 1
            self._matchStrs[look_for] = pos

        return prefix + str(self._matchStrs[look_for])

    def _compileCmd(self, tok):
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
            return '(tok->str()==' + self._insertMatchStr('|') + ')/* | */'
        elif tok == '%oror%':
            return '(tok->str()==' + self._insertMatchStr('||') + ')/* || */'
        elif tok == '%str%':
            return '(tok->type()==Token::eString)'
        elif tok == '%type%':
            return '(tok->isName() && tok->varId()==0U && tok->str() != ' + self._insertMatchStr('delete') + '/* delete */)'
        elif tok == '%var%':
            return 'tok->isName()'
        elif tok == '%varid%':
            return '(tok->isName() && tok->varId()==varid)'
        elif (len(tok)>2) and (tok[0]=="%"):
            print ("unhandled:" + tok)

        return '(tok->str()==' + self._insertMatchStr(tok) + ')/* ' + tok + ' */'

    def _compilePattern(self, pattern, nr, varid, isFindMatch=False):
        ret = ''
        returnStatement = ''

        if isFindMatch:
            ret = '\nconst Token *tok = start_tok;\n'
            returnStatement = 'continue;\n'
        else:
            arg2 = ''
            if varid:
                arg2 = ', const unsigned int varid'

            ret = '// ' + pattern + '\n'
            ret += 'static bool match' + str(nr) + '(const Token *tok'+arg2+') {\n'
            returnStatement = 'return false;\n'

        tokens = pattern.split(' ')
        gotoNextToken = ''
        checked_varid = False
        for tok in tokens:
            if tok == '':
                continue
            ret += gotoNextToken
            gotoNextToken = '    tok = tok->next();\n'

            # if varid is provided, check that it's non-zero on first use
            if varid and tok.find('%varid%') != -1 and checked_varid == False:
                ret += '    if (varid==0U)\n'
                ret += '        throw InternalError(tok, "Internal error. Token::Match called with varid 0. Please report this to Cppcheck developers");\n'
                checked_varid = True

            # [abc]
            if (len(tok) > 2) and (tok[0] == '[') and (tok[-1] == ']'):
                ret += '    if (!tok || tok->str().size()!=1U || !strchr("'+tok[1:-1]+'", tok->str()[0]))\n'
                ret += '        ' + returnStatement

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
                    ret += neg + self._compileCmd(tok2)

                if "" in tokens2:
                    ret += '))\n'
                    ret += '        tok = tok->next();\n'
                    gotoNextToken = ''
                else:
                    ret += '))\n'
                    ret += '        ' + returnStatement

            # !!a
            elif tok[0:2] == "!!":
                ret += '    if (tok && tok->str() == ' + self._insertMatchStr(tok[2:]) + ')/* ' + tok[2:] + ' */\n'
                ret += '        ' + returnStatement
                gotoNextToken = '    tok = tok ? tok->next() : NULL;\n'

            else:
                ret += '    if (!tok || !' + self._compileCmd(tok) + ')\n'
                ret += '        ' + returnStatement

        if isFindMatch:
            ret += '    return start_tok;\n'
        else:
            ret += '    return true;\n'
            ret += '}\n'

        return ret

    def _compileFindPattern(self, pattern, findmatchnr, endToken, varId):
        more_args = ''
        endCondition = ''
        if endToken:
            more_args += ', const Token *end'
            endCondition = ' && start_tok != end'
        if varId:
            more_args += ', unsigned int varid'

        ret = '// ' + pattern + '\n'
        ret += 'static const Token *findmatch' + str(findmatchnr) + '(const Token *start_tok'+more_args+') {\n'
        ret += '    for (; start_tok' + endCondition + '; start_tok = start_tok->next()) {\n'

        ret += self._compilePattern(pattern, -1, varId, True)
        ret += '    }\n'
        ret += '    return NULL;\n}\n'

        return ret

    def parseMatch(self, line, pos1):
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

    def _parseStringComparison(self, line, pos1):
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

    def _replaceTokenMatch(self, line):
        while True:
            pos1 = line.find('Token::Match(')
            if pos1 == -1:
                pos1 = line.find('Token::simpleMatch(')
            if pos1 == -1:
                break

            res = self.parseMatch(line, pos1)
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
                    patternNumber = len(self._matchFunctions) + 1
                    line = line[:pos1]+'match'+str(patternNumber)+'('+arg1+a3+')'+line[pos1+len(g0):]
                    self._matchFunctions.append(self._compilePattern(arg2, patternNumber, arg3))

        return line

    def _replaceTokenFindMatch(self, line):
        pos1 = 0
        while True:
            is_findmatch = False
            pos1 = line.find('Token::findsimplematch(')
            if pos1 == -1:
                is_findmatch = True
                pos1 = line.find('Token::findmatch(')
            if pos1 == -1:
                break

            res = self.parseMatch(line, pos1)
            if res == None:
                break
            else:
                assert(len(res)>=3 or len(res) < 6)  # assert that Token::find(simple)match has either 2, 3 or four arguments

                g0 = res[0]
                arg1 = res[1]
                pattern = res[2]

                # Check for varId
                varId = None
                if is_findmatch and g0.find("%varid%") != -1:
                    if len(res) == 5:
                        varId = res[4]
                    else:
                        varId = res[3]

                # endToken support. We resolve the overloaded type by checking if varId is used or not.
                # Function protoypes:
                #     Token *findsimplematch(const Token *tok, const char pattern[]);
                #     Token *findsimplematch(const Token *tok, const char pattern[], const Token *end);
                #     Token *findmatch(const Token *tok, const char pattern[], unsigned int varId = 0);
                #     Token *findmatch(const Token *tok, const char pattern[], const Token *end, unsigned int varId = 0);
                endToken = None
                if is_findmatch == False and len(res) == 4:
                    endToken = res[3]
                elif is_findmatch == True:
                    if varId and len(res) == 5:
                        endToken = res[3]
                    elif varId == None and len(res) == 4:
                        endToken = res[3]

                res = re.match(r'\s*"([^"]*)"\s*$', pattern)
                if res == None:
                    break  # Non-const pattern - bailout
                else:
                    pattern = res.group(1)
                    a3 = ''
                    if endToken:
                        a3 += ',' + endToken
                    if varId:
                        a3 += ',' + varId
                    findMatchNumber = len(self._matchFunctions) + 1
                    line = line[:pos1]+'findmatch'+str(findMatchNumber)+'('+arg1+a3+')'+line[pos1+len(g0):]
                    self._matchFunctions.append(self._compileFindPattern(pattern, findMatchNumber, endToken, varId))

        return line

    def _replaceCStrings(self, line):
        while True:
            match = re.search('str\(\) (==|!=) "', line)
            if not match:
                match = re.search('strAt\(.+?\) (==|!=) "', line)
            if not match:
                break

            res = self._parseStringComparison(line, match.start())
            if res == None:
                break

            startPos = res[0]
            endPos = res[1]
            text = line[startPos+1:endPos-1]
            line = line[:startPos] + self._insertMatchStr(text) + line[endPos:]

        return line

    def convertFile(self, srcname, destname):
        self._reset()

        fin = open(srcname, "rt")
        srclines = fin.readlines()
        fin.close()

        header = '#include "token.h"\n'
        header += '#include "errorlogger.h"\n'
        header += '#include <string>\n'
        header += '#include <cstring>\n'
        code = ''

        for line in srclines:
            # Compile Token::Match and Token::simpleMatch
            line = self._replaceTokenMatch(line)

            # Compile Token::findsimplematch
            # NOTE: Not enabled for now since the generated code is slower than before.
            # line = self._replaceTokenFindMatch(line)

            # Cache plain C-strings in C++ strings
            line = self._replaceCStrings(line)

            code += line

        # Compute string list
        stringList = ''
        for match in sorted(self._matchStrs, key=self._matchStrs.get):
            stringList += 'static const std::string matchStr' + str(self._matchStrs[match]) + '("' + match + '");\n'

        # Compute matchFunctions
        strFunctions = ''
        for function in self._matchFunctions:
            strFunctions += function

        fout = open(destname, 'wt')
        fout.write(header+stringList+strFunctions+code)
        fout.close()

    def _assertEquals(self, actual, expected):
        if actual != expected:
            print ('Assertion failed:')
            print (actual)
            print (expected)
            assert actual == expected

    def _selftests(self):
        self._assertEquals(self.parseMatch('  Token::Match(tok, ";") ',2), ['Token::Match(tok, ";")','tok',' ";"'])
        self._assertEquals(self.parseMatch('  Token::Match(tok,', 2), None) # multiline Token::Match is not supported yet
        self._assertEquals(self.parseMatch('  Token::Match(Token::findsimplematch(tok,")"), ";")', 2), ['Token::Match(Token::findsimplematch(tok,")"), ";")', 'Token::findsimplematch(tok,")")', ' ";"']) # inner function call


# Main program
mc = MatchCompiler()

# convert all lib/*.cpp files
for f in glob.glob('lib/*.cpp'):
    print (f + ' => build/' + f[4:])
    mc.convertFile(f, 'build/'+f[4:])
