
import os
import re

def hasresult(filename, result):
  if not os.path.isfile(filename):
    return False
  for line in open(filename, 'rt'):
    if line.find(result) >= 0:
      return True
  return False

def parsefile(filename):
  ret = []
  linenr = 0
  functionName = None
  for line in open(filename,'rt'):
    linenr = linenr + 1
    res = re.match('^[a-z]+[ *]+([a-z0-9_]+)[(]', line)
    if res:
      functionName = res.group(1)
    if line.startswith('}'):
      functionName = ''
    elif line.find('BUG')>0 or line.find('WARN')>0 or filename=='ub.c':
      spaces = ''
      for i in range(100):
        spaces = spaces + ' '
      s = filename + spaces
      s = s[:15] + str(linenr) + spaces
      s = s[:20] + functionName + spaces
      s = s[:50]
      if hasresult('cppcheck.txt', '[' + filename + ':' + str(linenr) + ']'):
        s = s + '      X'
      else:
        s = s + '       '
      if hasresult('clang.txt', filename + ':' + str(linenr)):
        s = s + '      X'
      else:
        s = s + '       '
      if hasresult('lint.txt', filename + '  ' + str(linenr)):
        s = s + '      X'
      else:
        s = s + '       '
      if hasresult('cov.txt', filename + ':' + str(linenr)):
        s = s + '      X'
      else:
        s = s + '       '
      ret.append(s)
  return ret

bugs = []
bugs.extend(parsefile('controlflow.c'))
bugs.extend(parsefile('data.c'))
bugs.extend(parsefile('functions.c'))
bugs.extend(parsefile('ub.c'))
for bug in bugs:
  print(bug)
