#!/usr/bin/python

# cgi-script for searching the results

import sys
import glob
import os
import cgi
import cgitb
import re

def getfiles(path, arguments):
  files = []
  if 'folder' in arguments:
    files.append(path + '/daca2-' + arguments['folder'].value + '.html')
  else:
    files.extend(sorted(glob.glob(path+'/daca2-?.html')))
    files.extend(sorted(glob.glob(path+'/daca2-lib?.html')))
  return files

def readlines(filename):
  if not os.path.isfile(filename):
    return []
  f = open(filename, 'rt')
  lines = f.readlines()
  f.close()
  return lines

def trimline(line):
  while len(line)>1 and (line[-1]=='\r' or line[-1]=='\n'):
    line = line[:-1]
  return line

def matchline(line, id):
  return line.endswith('[' + id + ']')

def doSearch(path,arguments):
  id = arguments['id'].value
  for g in getfiles(path, arguments):
    ftp = ''
    found = False
    for line in readlines(g):
      line = trimline(line)
      if line.startswith('ftp://'):
        ftp = line
      if matchline(line, id):
        found = True
        sys.stdout.write(ftp + '\n')
      elif line.find(': note:') < 0:
        found = False
      if found:
        sys.stdout.write(line + '\n')

def summary(path, arguments):
  count = {}
  pattern = re.compile(r'.*: (error|warning|style|performance|portability):.*\[([a-zA-Z0-9]+)\]$')
  for g in getfiles(path, arguments):
    for line in readlines(g):
      res = pattern.match(trimline(line))
      if res is None:
        continue
      id = res.group(2)
      if id in count:
        count[id] += 1
      else:
        count[id] = 1
  print('<table>')
  for id in sorted(count.keys()):
    print('<tr><td>' + id +'</td><td><a href="/cgi-bin/daca2-search.cgi?id='+id+'">'+str(count[id])+'</a></td></tr>')
  print('</table>')

sys.stdout.write('Content-type: text/html\r\n\r\n'
                 '<html><body>\n')

cgitb.enable()
arguments = cgi.FieldStorage()
if 'id' in arguments:
  id = arguments['id'].value
  #id = 'oppositeInnerCondition'
  print(id)
  sys.stdout.write('<pre>\n')
  doSearch('../htdocs/devinfo/daca2-report', arguments)
  #doSearch(os.path.expanduser('~/temp'), id)
  sys.stdout.write('</pre>\n')
else:
  summary('../htdocs/devinfo/daca2-report', arguments)
  #summary(os.path.expanduser('~/temp'), arguments)
sys.stdout.write('</body></html>\n')
