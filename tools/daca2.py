
#
# 1. Create a folder daca2 in your HOME folder
# 2. Put cppcheck-O2 in daca2. It should be built with all optimisations.
# 3. Edit this line:   FOLDER = 'a'
# 4. Optional: Put a file called "suppressions.txt" in the daca2 folder.
# 5. Run the daca2 script:  python daca2.py
#

import ftplib
import subprocess
import sys
import shutil
import glob
import os
import socket

FOLDER = 'a'

def removeAllExceptResults():
  filenames = glob.glob('[A-Za-z]*')
  for filename in filenames:
    if os.path.isdir(filename):
      shutil.rmtree(filename)
    elif filename != 'results.txt':
      os.remove(filename)

def getpackages(c):
  f = ftplib.FTP('ftp.sunet.se','anonymous','','',60)
  f.login('anonymous','password')
  packages = f.nlst('/pub/Linux/distributions/Debian/debian/pool/main/' + c)
  f.quit()
  return packages

workdir = os.path.expanduser('~/daca2/')
if not os.path.isfile(workdir + 'suppressions.txt'):
  suppressions = open(workdir + 'suppressions.txt', 'wt')
  suppressions.write('\n')
  suppressions.close()

if not os.path.isdir(workdir + FOLDER):
  os.makedirs(workdir + FOLDER)
os.chdir(workdir + FOLDER)
if os.path.isfile('results.txt'):
  os.remove('results.txt')

packages = getpackages(FOLDER)

for package in packages:
  filename = None
  path = '/pub/Linux/distributions/Debian/debian/pool/main/' + FOLDER + '/' + package
  f = None
  try:
    f = ftplib.FTP('ftp.sunet.se','anonymous','password','',60)
    f.login('anonymous','x')
    for s in f.nlst(path):
      if s[-12:] == '.orig.tar.gz':
        filename = s
  except socket.error:
    pass
  except ftplib.error_temp:
    pass
    if f:
      try:
        f.quit()
      except ftplib.error_reply:
        pass

    if filename:
      fullpath = 'ftp://ftp.sunet.se' + path + '/' + filename
      subprocess.call(['wget',fullpath])
      subprocess.call(['tar', 'xzvf', filename])
      subprocess.call(['rm', filename])

      dirname = None
      for s in glob.glob(filename[:2] + '*'):
        if os.path.isdir(s):
          dirname = s
      if dirname is None:
        continue

      print('cppcheck "' + dirname + '"')
      p = subprocess.Popen(['nice', '../cppcheck-O2', '-j2', '-D__GCC__', '--enable=style', '--suppressions-list=../suppressions.txt', dirname], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
      comm = p.communicate()

      results = open('results.txt', 'at')
      results.write(comm[1] + '\n')
      results.close()

      # remove all files/folders except results.txt
      removeAllExceptResults()
