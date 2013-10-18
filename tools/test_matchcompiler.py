#!/usr/bin/python
#
# Cppcheck - A tool for static C/C++ code analysis
# Copyright (C) 2007-2013 Daniel Marjamaeki and Cppcheck team.
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

import unittest
import matchcompiler


class MatchCompilerTest(unittest.TestCase):

    def setUp(self):
        self.mc = matchcompiler.MatchCompiler(verify_mode=False)

    def test_parseMatch(self):
        self.assertEqual(self.mc.parseMatch('  Token::Match(tok, ";") ', 2), [
                         'Token::Match(tok, ";")', 'tok', ' ";"'])
        self.assertEqual(self.mc.parseMatch('  Token::Match(tok,', 2), None)
                         # multiline Token::Match is not supported yet
        self.assertEqual(self.mc.parseMatch('  Token::Match(Token::findsimplematch(tok,")"), ";")', 2), [
                         'Token::Match(Token::findsimplematch(tok,")"), ";")', 'Token::findsimplematch(tok,")")', ' ";"'])  # inner function call

    def test_replaceTokenMatch(self):
        input = 'if (Token::Match(tok, "foobar")) {'
        output = self.mc._replaceTokenMatch(input)
        self.assertEqual(output, 'if (match1(tok)) {')
        self.assertEqual(1, len(self.mc._matchStrs))
        self.assertEqual(1, self.mc._matchStrs['foobar'])

        input = 'if (Token::Match(tok->next()->next(), "foobar %type% %num%")) {'
        output = self.mc._replaceTokenMatch(input)
        self.assertEqual(output, 'if (match2(tok->next()->next())) {')
        self.assertEqual(2, len(self.mc._matchStrs))

        input = 'if (Token::Match(tok, "foo\"special\"bar %num%")) {'
        output = self.mc._replaceTokenMatch(input)
        # FIXME: Currently detected as non-static pattern
        self.assertEqual(
            output, 'if (Token::Match(tok, "foo"special"bar %num%")) {')
        # self.assertEqual(3, len(self.mc._matchStrs))

    def test_replaceTokenMatchWithVarId(self):
        input = 'if (Token::Match(tok, "foobar %varid%", 123)) {'
        output = self.mc._replaceTokenMatch(input)
        self.assertEqual(output, 'if (match1(tok, 123)) {')
        self.assertEqual(1, len(self.mc._matchStrs))

        input = 'if (Token::Match(tok->next()->next(), "%varid% foobar", tok->varId())) {'
        output = self.mc._replaceTokenMatch(input)
        self.assertEqual(
            output, 'if (match2(tok->next()->next(), tok->varId())) {')
        self.assertEqual(1, len(self.mc._matchStrs))

        input = 'if (Token::Match(tok, "foo\"special\"bar %type% %varid%", my_varid_cache)) {'
        output = self.mc._replaceTokenMatch(input)
        # FIXME: Currently detected as non-static pattern
        self.assertEqual(
            output, 'if (Token::Match(tok, "foo"special"bar %type% %varid%", my_varid_cache)) {')
        # self.assertEqual(1, len(self.mc._matchStrs))

        # test caching: reuse existing matchX()
        input = 'if (Token::Match(tok, "foobar %varid%", 123)) {'
        output = self.mc._replaceTokenMatch(input)
        self.assertEqual(output, 'if (match1(tok, 123)) {')
        self.assertEqual(1, len(self.mc._matchStrs))

        # two in one line
        input = 'if (Token::Match(tok, "foobar2 %varid%", 123) || Token::Match(tok, "%type% %varid%", 123)) {'
        output = self.mc._replaceTokenMatch(input)
        self.assertEqual(output, 'if (match3(tok, 123) || match4(tok, 123)) {')
        self.assertEqual(3, len(self.mc._matchStrs))

    def test_replaceTokenSimpleMatch(self):
        input = 'if (Token::simpleMatch(tok, "foobar")) {'
        output = self.mc._replaceTokenMatch(input)
        self.assertEqual(output, 'if (match1(tok)) {')
        self.assertEqual(1, len(self.mc._matchStrs))
        self.assertEqual(1, self.mc._matchStrs['foobar'])

        input = 'if (Token::simpleMatch(tok->next()->next(), "foobar")) {'
        output = self.mc._replaceTokenMatch(input)
        self.assertEqual(output, 'if (match1(tok->next()->next())) {')
        self.assertEqual(1, len(self.mc._matchStrs))
        self.assertEqual(1, self.mc._matchStrs['foobar'])

        input = 'if (Token::simpleMatch(tok, "foo\"special\"bar")) {'
        output = self.mc._replaceTokenMatch(input)
        # FIXME: Currently detected as non-static pattern
        self.assertEqual(
            output, 'if (Token::simpleMatch(tok, "foo\"special\"bar")) {')
        self.assertEqual(1, len(self.mc._matchStrs))

    def test_replaceTokenFindSimpleMatch(self):
        input = 'if (Token::findsimplematch(tok, "foobar")) {'
        output = self.mc._replaceTokenFindMatch(input)
        self.assertEqual(output, 'if (findmatch1(tok)) {')
        self.assertEqual(1, len(self.mc._matchStrs))
        self.assertEqual(1, self.mc._matchStrs['foobar'])

        input = 'if (Token::findsimplematch(tok->next()->next(), "foobar", tok->link())) {'
        output = self.mc._replaceTokenFindMatch(input)
        self.assertEqual(
            output, 'if (findmatch2(tok->next()->next(), tok->link())) {')
        self.assertEqual(1, len(self.mc._matchStrs))
        self.assertEqual(1, self.mc._matchStrs['foobar'])

        input = 'if (Token::findsimplematch(tok, "foo\"special\"bar")) {'
        output = self.mc._replaceTokenFindMatch(input)
        # FIXME: Currently detected as non-static pattern
        self.assertEqual(
            output, 'if (Token::findsimplematch(tok, "foo\"special\"bar")) {')
        self.assertEqual(1, len(self.mc._matchStrs))

    def test_replaceTokenFindMatch(self):
        input = 'if (Token::findmatch(tok, "foobar")) {'
        output = self.mc._replaceTokenFindMatch(input)
        self.assertEqual(output, 'if (findmatch1(tok)) {')
        self.assertEqual(1, len(self.mc._matchStrs))
        self.assertEqual(1, self.mc._matchStrs['foobar'])

        # findmatch with varid
        input = 'if (Token::findmatch(tok, "foobar %varid%", tok->varId())) {'
        output = self.mc._replaceTokenFindMatch(input)
        self.assertEqual(output, 'if (findmatch2(tok, tok->varId())) {')
        self.assertEqual(1, len(self.mc._matchStrs))
        self.assertEqual(1, self.mc._matchStrs['foobar'])

        # findmatch with end token
        input = 'if (Token::findmatch(tok->next()->next(), "foobar %type%", tok->link())) {'
        output = self.mc._replaceTokenFindMatch(input)
        self.assertEqual(
            output, 'if (findmatch3(tok->next()->next(), tok->link())) {')
        self.assertEqual(2, len(self.mc._matchStrs))
        self.assertEqual(1, self.mc._matchStrs['foobar'])

        # findmatch with end token and varid
        input = 'if (Token::findmatch(tok->next()->next(), "foobar %type% %varid%", tok->link(), 123)) {'
        output = self.mc._replaceTokenFindMatch(input)
        self.assertEqual(
            output, 'if (findmatch4(tok->next()->next(), tok->link(), 123)) {')
        self.assertEqual(2, len(self.mc._matchStrs))
        self.assertEqual(1, self.mc._matchStrs['foobar'])

if __name__ == '__main__':
    unittest.main()
