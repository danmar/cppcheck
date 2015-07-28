#/usr/bin/python
# cppcheck addon for naming conventions

import cppcheckdata
import sys
import re

RE_VARNAME = None
RE_FUNCTIONNAME = None
for arg in sys.argv[1:]:
  if arg[:6]=='--var=':
    RE_VARNAME = arg[6:]
  elif arg[:11]=='--function=':
    RE_FUNCTIONNAME = arg[11:]

def reportError(token, severity, msg):
  sys.stderr.write('[' + token.file + ':' + str(token.linenr) + '] (' + severity + ') ' + msg + '\n')

for arg in sys.argv[1:]:
  if not arg[-5:]=='.dump':
    continue
  print('Checking ' + arg + '...')
  data = cppcheckdata.parsedump(arg)
  if RE_VARNAME:
    for var in data.variables:
      res = re.match(RE_VARNAME, var.nameToken.str)
      if not res:
        reportError(var.typeStartToken, 'style', 'Variable ' + var.nameToken.str + ' violates naming convention')
  if RE_FUNCTIONNAME:
    for function in data.functions:
      res = re.match(RE_FUNCTIONNAME, function.name)
      if not res:
        reportError(var.typeStartToken, 'style', 'Function ' + function.name + ' violates naming convention')
