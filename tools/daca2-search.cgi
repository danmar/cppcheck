#!/usr/bin/python

# cgi-script for searching the results

import sys
import glob
import os
import cgi
import cgitb

def matchline(line, id):
  return line.endswith('[' + id + ']')

def doSearch(path,arguments):
  id = arguments['id'].value

  files = []
  if 'folder' in arguments:
    files.append(path + '/daca2-' + arguments['folder'].value + '.html')
  else:
    files.extend(sorted(glob.glob(path+'/daca2-?.html')))
    files.extend(sorted(glob.glob(path+'/daca2-lib?.html')))

  for g in files:
    if os.path.isfile(g):
      ftp = ''
      found = False
      f = open(g,'rt')
      for line in f.readlines():
        while len(line)>1 and (line[-1]=='\r' or line[-1]=='\n'):
          line = line[:-1]
        if line.startswith('ftp://'):
          ftp = line
        if matchline(line, id):
          found = True
          sys.stdout.write(ftp + '\n')
        elif line.find(': note:') < 0:
          found = False
        if found:
          sys.stdout.write(line + '\n')
      f.close()

sys.stdout.write('Content-type: text/html\r\n\r\n')

sys.stdout.write('<html><body><pre>\n')

cgitb.enable()
arguments = cgi.FieldStorage()
id = arguments['id'].value
#id = 'oppositeInnerCondition'
print(id)

doSearch('../htdocs/devinfo/daca2-report', arguments)
#doSearch(os.path.expanduser('~/temp'), id)

sys.stdout.write('</pre></body></html>\n')
