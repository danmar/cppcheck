#!/usr/bin/env python
#
# Locate casts in the code
#

import cppcheckdata
import sys

messages = set()

for arg in sys.argv[1:]:
    print('Checking ' + arg + '...')
    data = cppcheckdata.parsedump(arg)

    for cfg in data.configurations:
        if len(data.configurations) > 1:
            print('Checking ' + arg + ', config "' + cfg.name + '"...')
        for token in cfg.tokenlist:
            if token.str != '(' or not token.astOperand1 or token.astOperand2:
                continue

            # we probably have a cast.. if there is something inside the parentheses
            # there is a cast. Otherwise this is a function call.
            typetok = token.next
            if not typetok.isName:
                continue

            # cast number => skip output
            if token.astOperand1.isNumber:
                continue

            # void cast => often used to suppress compiler warnings
            if typetok.str == 'void':
                continue

            msg = '[' + token.file + ':' + str(
                token.linenr) + '] (information) findcasts.py: found a cast\n'
            if msg not in messages:
                messages.add(msg)
                sys.stderr.write(msg)
