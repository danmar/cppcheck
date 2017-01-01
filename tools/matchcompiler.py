#!/usr/bin/python
#
# Cppcheck - A tool for static C/C++ code analysis
# Copyright (C) 2007-2016 Cppcheck team.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import io
import os
import sys
import re
import glob
import argparse


class MatchCompiler:

    def __init__(self, verify_mode=False, show_skipped=False):
        self._verifyMode = verify_mode
        self._showSkipped = show_skipped
        self._reset()

    def _reset(self):
        self._rawMatchFunctions = []
        self._matchFunctionCache = {}

    def _generateCacheSignature(
            self, pattern, endToken=None, varId=None, isFindMatch=False):
        sig = pattern

        if endToken:
            sig += '|ENDTOKEN'
        else:
            sig += '|NO-ENDTOKEN'

        if varId:
            sig += '|VARID'
        else:
            sig += '|NO-VARID'

        if isFindMatch:
            sig += '|ISFINDMATCH'
        else:
            sig += '|NORMALMATCH'

        return sig

    def _lookupMatchFunctionId(
            self, pattern, endToken=None, varId=None, isFindMatch=False):
        signature = self._generateCacheSignature(
            pattern, endToken, varId, isFindMatch)

        if signature in self._matchFunctionCache:
            return self._matchFunctionCache[signature]

        return None

    def _insertMatchFunctionId(
            self, id, pattern, endToken=None, varId=None, isFindMatch=False):
        signature = self._generateCacheSignature(
            pattern, endToken, varId, isFindMatch)

        # function signature should not be in the cache
        assert(
            self._lookupMatchFunctionId(
                pattern,
                endToken,
                varId,
                isFindMatch) is None)

        self._matchFunctionCache[signature] = id

    def _compileCmd(self, tok):
        if tok == '%any%':
            return 'true'
        elif tok == '%assign%':
            return 'tok->isAssignmentOp()'
        elif tok == '%bool%':
            return 'tok->isBoolean()'
        elif tok == '%char%':
            return '(tok->tokType()==Token::eChar)'
        elif tok == '%comp%':
            return 'tok->isComparisonOp()'
        elif tok == '%num%':
            return 'tok->isNumber()'
        elif tok == '%cop%':
            return 'tok->isConstOp()'
        elif tok == '%op%':
            return 'tok->isOp()'
        elif tok == '%or%':
            return '(tok->tokType() == Token::eBitOp && tok->str()==MatchCompiler::makeConstString("|") )'
        elif tok == '%oror%':
            return '(tok->tokType() == Token::eLogicalOp && tok->str()==MatchCompiler::makeConstString("||"))'
        elif tok == '%str%':
            return '(tok->tokType()==Token::eString)'
        elif tok == '%type%':
            return '(tok->isName() && tok->varId()==0U && !tok->isKeyword())'
        elif tok == '%name%':
            return 'tok->isName()'
        elif tok == '%var%':
            return '(tok->varId() != 0)'
        elif tok == '%varid%':
            return '(tok->isName() && tok->varId()==varid)'
        elif (len(tok) > 2) and (tok[0] == "%"):
            print("unhandled:" + tok)

        return (
            '(tok->str()==MatchCompiler::makeConstString("' + tok + '"))'
        )

    def _compilePattern(self, pattern, nr, varid,
                        isFindMatch=False, tokenType="const Token"):
        ret = ''
        returnStatement = ''

        if isFindMatch:
            ret = '\n    ' + tokenType + ' * tok = start_tok;\n'
            returnStatement = 'continue;\n'
        else:
            arg2 = ''
            if varid:
                arg2 = ', const unsigned int varid'

            ret = '// pattern: ' + pattern + '\n'
            ret += 'static bool match' + \
                str(nr) + '(' + tokenType + '* tok' + arg2 + ') {\n'
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
            if varid and tok.find('%varid%') != -1 and checked_varid is False:
                ret += '    if (varid==0U)\n'
                ret += '        throw InternalError(tok, "Internal error. Token::Match called with varid 0. Please report this to Cppcheck developers");\n'
                checked_varid = True

            # [abc]
            if (len(tok) > 2) and (tok[0] == '[') and (tok[-1] == ']'):
                ret += '    if (!tok || tok->str().size()!=1U || !strchr("' + tok[1:-1] + '", tok->str()[0]))\n'
                ret += '        ' + returnStatement

            # a|b|c
            elif tok.find('|') > 0:
                tokens2 = tok.split('|')
                logicalOp = ' || '
                if "" in tokens2:
                    ret += '    if (tok && ('
                else:
                    ret += '    if (!tok || !('
                first = True
                for tok2 in tokens2:
                    if tok2 == '':
                        continue
                    if not first:
                        ret += logicalOp
                    first = False
                    ret += self._compileCmd(tok2)

                ret += '))\n'
                if "" in tokens2:
                    ret += '        tok = tok->next();\n'
                    gotoNextToken = ''
                else:
                    ret += '        ' + returnStatement

            # !!a
            elif tok[0:2] == "!!":
                ret += '    if (tok && tok->str() == MatchCompiler::makeConstString("' + tok[2:] + '"))\n'
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
            more_args += ', const Token * end'
            endCondition = ' && start_tok != end'
        if varId:
            more_args += ', unsigned int varid'

        ret = '// pattern: ' + pattern + '\n'
        ret += 'template<class T> static T * findmatch' + \
            str(findmatchnr) + '(T * start_tok' + more_args + ') {\n'
        ret += '    for (; start_tok' + endCondition + \
            '; start_tok = start_tok->next()) {\n'

        ret += self._compilePattern(pattern, -1, varId, True, 'T')
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
                    ret.append(line[pos1:pos + 1])
                    for arg in args:
                        ret.append(arg)
                    ret.append(line[argstart:pos])
                    return ret
            elif line[pos] == ',' and parlevel == 1:
                args.append(line[argstart:pos])
                argstart = pos + 1
            pos += 1

        return None

    def _isInString(self, line, pos1):
        pos = 0
        inString = False
        while pos != pos1:
            if line[pos] == '\\':
                pos += 1
            elif line[pos] == '"':
                inString = not inString
            pos += 1
        return inString

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
                    endPos = pos + 1
                    return (startPos, endPos)
            elif line[pos] == '"':
                startPos = pos
                inString = True
            pos += 1

        return None

    def _compileVerifyTokenMatch(
            self, is_simplematch, verifyNumber, pattern, patternNumber, varId):
        more_args = ''
        if varId:
            more_args = ', const unsigned int varid'

        ret = 'static bool match_verify' + \
            str(verifyNumber) + '(const Token *tok' + more_args + ') {\n'

        origMatchName = 'Match'
        if is_simplematch:
            origMatchName = 'simpleMatch'
            assert(varId is None)

        ret += '    bool res_compiled_match = match' + \
            str(patternNumber) + '(tok'
        if varId:
            ret += ', varid'
        ret += ');\n'

        ret += '    bool res_parsed_match = Token::' + \
            origMatchName + '(tok, "' + pattern + '"'
        if varId:
            ret += ', varid'
        ret += ');\n'

        ret += '\n'
        # Don't use assert() here, it's disabled for optimized builds.
        # We also need to verify builds in 'release' mode
        ret += '    if (res_parsed_match != res_compiled_match) {\n'
#        ret += '        std::cout << "res_parsed_match' + str(verifyNumber) + ': " << res_parsed_match << ", res_compiled_match: " << res_compiled_match << "\\n";\n'
#        ret += '        if (tok)\n'
#        ret += '            std::cout << "tok: " << tok->str();\n'
#        ret += '        if (tok->next())\n'
#        ret += '            std::cout << "tok next: " << tok->next()->str();\n'
        ret += '        throw InternalError(tok, "Internal error. compiled match returned different result than parsed match: ' + pattern + '");\n'
        ret += '    }\n'
        ret += '    return res_compiled_match;\n'
        ret += '}\n'

        return ret

    def _replaceSpecificTokenMatch(
            self, is_simplematch, line, start_pos, end_pos, pattern, tok, varId):
        more_args = ''
        if varId:
            more_args = ',' + varId

        # Compile function or use previously compiled one
        patternNumber = self._lookupMatchFunctionId(
            pattern, None, varId, False)

        if patternNumber is None:
            patternNumber = len(self._rawMatchFunctions) + 1
            self._insertMatchFunctionId(
                patternNumber,
                pattern,
                None,
                varId,
                False)
            self._rawMatchFunctions.append(
                self._compilePattern(pattern, patternNumber, varId))

        functionName = "match"
        if self._verifyMode:
            verifyNumber = len(self._rawMatchFunctions) + 1
            self._rawMatchFunctions.append(
                self._compileVerifyTokenMatch(
                    is_simplematch,
                    verifyNumber,
                    pattern,
                    patternNumber,
                    varId))

            # inject verify function
            functionName = "match_verify"
            patternNumber = verifyNumber

        return (
            line[:start_pos] + functionName + str(
                patternNumber) + '(' + tok + more_args + ')' + line[start_pos + end_pos:]
        )

    def _replaceTokenMatch(self, line, linenr, filename):
        while True:
            is_simplematch = False
            pos1 = line.find('Token::Match(')
            if pos1 == -1:
                is_simplematch = True
                pos1 = line.find('Token::simpleMatch(')
            if pos1 == -1:
                break

            res = self.parseMatch(line, pos1)
            if res is None:
                break

            # assert that Token::Match has either 2 or 3 arguments
            assert(len(res) == 3 or len(res) == 4)

            end_pos = len(res[0])
            tok = res[1]
            raw_pattern = res[2]
            varId = None
            if len(res) == 4:
                varId = res[3]

            res = re.match(r'\s*"((?:.|\\")*?)"\s*$', raw_pattern)
            if res is None:
                if self._showSkipped:
                    print(filename +":" + str(linenr) +" skipping match pattern:" + raw_pattern)
                break  # Non-const pattern - bailout

            pattern = res.group(1)
            line = self._replaceSpecificTokenMatch(
                is_simplematch,
                line,
                pos1,
                end_pos,
                pattern,
                tok,
                varId)

        return line

    def _compileVerifyTokenFindMatch(
            self, is_findsimplematch, verifyNumber, pattern, patternNumber, endToken, varId):
        more_args = ''
        if endToken:
            more_args += ', const Token * endToken'
        if varId:
            more_args += ', const unsigned int varid'

        ret = 'template < class T > static T * findmatch_verify' + \
            str(verifyNumber) + '(T * tok' + more_args + ') {\n'

        origFindMatchName = 'findmatch'
        if is_findsimplematch:
            origFindMatchName = 'findsimplematch'
            assert(varId is None)

        ret += '    T * res_compiled_findmatch = findmatch' + \
            str(patternNumber) + '(tok'
        if endToken:
            ret += ', endToken'
        if varId:
            ret += ', varid'
        ret += ');\n'

        ret += '    T * res_parsed_findmatch = Token::' + \
            origFindMatchName + '(tok, "' + pattern + '"'
        if endToken:
            ret += ', endToken'
        if varId:
            ret += ', varid'
        ret += ');\n'

        ret += '\n'
        # Don't use assert() here, it's disabled for optimized builds.
        # We also need to verify builds in 'release' mode
        ret += '    if (res_parsed_findmatch != res_compiled_findmatch) {\n'
        ret += '        throw InternalError(tok, "Internal error. compiled findmatch returned different result than parsed findmatch: ' + pattern + '");\n'
        ret += '    }\n'
        ret += '    return res_compiled_findmatch;\n'
        ret += '}\n'

        return ret

    def _replaceSpecificFindTokenMatch(
            self, is_findsimplematch, line, start_pos, end_pos, pattern, tok, endToken, varId):
        more_args = ''
        if endToken:
            more_args += ',' + endToken
        if varId:
            more_args += ',' + varId

        # Compile function or use previously compiled one
        findMatchNumber = self._lookupMatchFunctionId(
            pattern, endToken, varId, True)

        if findMatchNumber is None:
            findMatchNumber = len(self._rawMatchFunctions) + 1
            self._insertMatchFunctionId(
                findMatchNumber,
                pattern,
                endToken,
                varId,
                True)
            self._rawMatchFunctions.append(
                self._compileFindPattern(
                    pattern,
                    findMatchNumber,
                    endToken,
                    varId))

        functionName = "findmatch"
        if self._verifyMode:
            verifyNumber = len(self._rawMatchFunctions) + 1
            self._rawMatchFunctions.append(
                self._compileVerifyTokenFindMatch(
                    is_findsimplematch,
                    verifyNumber,
                    pattern,
                    findMatchNumber,
                    endToken,
                    varId))

            # inject verify function
            functionName = "findmatch_verify"
            findMatchNumber = verifyNumber

        return (
            line[:start_pos] + functionName + str(
                findMatchNumber) + '(' + tok + more_args + ') ' + line[start_pos + end_pos:]
        )

    def _replaceTokenFindMatch(self, line, linenr, filename):
        pos1 = 0
        while True:
            is_findsimplematch = True
            pos1 = line.find('Token::findsimplematch(')
            if pos1 == -1:
                is_findsimplematch = False
                pos1 = line.find('Token::findmatch(')
            if pos1 == -1:
                break

            res = self.parseMatch(line, pos1)
            if res is None:
                break

            assert(len(res) >= 3 or len(res) < 6)
            # assert that Token::find(simple)match has either 2, 3 or 4
            # arguments

            g0 = res[0]
            tok = res[1]
            pattern = res[2]

            # Check for varId
            varId = None
            if not is_findsimplematch and g0.find("%varid%") != -1:
                if len(res) == 5:
                    varId = res[4]
                else:
                    varId = res[3]

            # endToken support. We resolve the overloaded type by checking if varId is used or not.
            # Function protoypes:
            #     Token *findsimplematch(const Token *tok, const char pattern[]);
            #     Token *findsimplematch(const Token *tok, const char pattern[], const Token *end);
            #     Token *findmatch(const Token *tok, const char pattern[], unsigned int varId = 0);
            # Token *findmatch(const Token *tok, const char pattern[], const
            # Token *end, unsigned int varId = 0);
            endToken = None
            if ((is_findsimplematch and len(res) == 4) or
               (not is_findsimplematch and varId and (len(res) == 5)) or
               (not is_findsimplematch and varId is None and len(res) == 4)):
                endToken = res[3]

            res = re.match(r'\s*"((?:.|\\")*?)"\s*$', pattern)
            if res is None:
                if self._showSkipped:
                    print(filename +":" + str(linenr) +" skipping findmatch pattern:" + pattern)
                break  # Non-const pattern - bailout

            pattern = res.group(1)
            line = self._replaceSpecificFindTokenMatch(
                is_findsimplematch,
                line,
                pos1,
                len(g0),
                pattern,
                tok,
                endToken,
                varId)

        return line

    def _replaceCStrings(self, line):
        while True:
            match = re.search('(==|!=) *"', line)
            if not match:
                break

            if self._isInString(line, match.start()):
                break

            res = self._parseStringComparison(line, match.start())
            if res is None:
                break

            startPos = res[0]
            endPos = res[1]
            text = line[startPos + 1:endPos - 1]
            line = line[:startPos] + 'MatchCompiler::makeConstStringBegin' + text + 'MatchCompiler::makeConstStringEnd' + line[endPos:]
        line = line.replace('MatchCompiler::makeConstStringBegin', 'MatchCompiler::makeConstString("')
        line = line.replace('MatchCompiler::makeConstStringEnd', '")')
        return line

    def convertFile(self, srcname, destname, line_directive):
        self._reset()

        fin = io.open(srcname, "rt", encoding="utf-8")
        srclines = fin.readlines()
        fin.close()

        header = '#include "token.h"\n'
        header += '#include "errorlogger.h"\n'
        header += '#include "matchcompiler.h"\n'
        header += '#include <string>\n'
        header += '#include <cstring>\n'
        # header += '#include <iostream>\n'
        code = ''

        linenr = 0
        for line in srclines:
            linenr += 1
            # Compile Token::Match and Token::simpleMatch
            line = self._replaceTokenMatch(line, linenr, srcname)

            # Compile Token::findsimplematch
            line = self._replaceTokenFindMatch(line, linenr, srcname)

            # Cache plain C-strings in C++ strings
            line = self._replaceCStrings(line)

            code += line

        # Compute matchFunctions
        strFunctions = ''
        for function in self._rawMatchFunctions:
            strFunctions += function

        lineno = ''
        if line_directive:
            lineno = '#line 1 "' + srcname + '"\n'

        fout = io.open(destname, 'wt', encoding="utf-8")
        fout.write(header + strFunctions + lineno + code)
        fout.close()


def main():
    # Main program

    # Argument handling
    parser = argparse.ArgumentParser(
        description='Compile Token::Match() calls into native C++ code')
    parser.add_argument('--verify', action='store_true', default=False,
                        help='verify compiled matches against on-the-fly parser. Slow!')
    parser.add_argument('--show-skipped', action='store_true', default=False,
                        help='show skipped (non-static) patterns')
    parser.add_argument('--read-dir', default="lib",
                        help='directory from which files are read')
    parser.add_argument('--write-dir', default="build",
                        help='directory into which files are written')
    parser.add_argument('--prefix', default="",
                        help='prefix for build files')
    parser.add_argument('--line', action='store_true', default=False,
                        help='add line directive to input files into build files')
    parser.add_argument('file', nargs='*',
                        help='file to complile')
    args = parser.parse_args()
    lib_dir = args.read_dir
    build_dir = args.write_dir
    line_directive = args.line
    files = args.file

    # Check if we are invoked from the right place
    if not os.path.exists(lib_dir):
        print('Directory "' + lib_dir + '"not found.')
        sys.exit(-1)

    # Create build directory if needed
    if not os.path.exists(build_dir):
        os.makedirs(build_dir)
    if not os.path.isdir(build_dir):
        raise Exception(build_dir + ' is not a directory')

    mc = MatchCompiler(verify_mode=args.verify,
                       show_skipped=args.show_skipped)

    if not files:
        # select all *.cpp files in lib_dir
        for f in glob.glob(lib_dir + '/*.cpp'):
            files.append(f[len(lib_dir) + 1:])

    # convert files
    for fi in files:
        pi = lib_dir + '/' + fi
        fo = args.prefix + fi
        po = build_dir + '/' + fo
        print(pi + ' => ' + po)
        mc.convertFile(pi, po, line_directive)

if __name__ == '__main__':
    main()
