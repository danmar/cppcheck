
import os
import sys

def readdate(data):
  datepos = -1
  if data[:5] == 'DATE ':
    datepos = 0
  else:
    datepos = data.find('\nDATE ')
    if datepos >= 0:
      datepos = datepos + 1

  if datepos < 0:
    return None

  datestr = ''
  datepos = datepos + 5
  while True:
    if datepos >= len(data):
      return None
    d = data[datepos]
    if d>='0' and d<='9':
      datestr = datestr + d
    elif d=='\n':
      if len(datestr) == 8:
        return datestr[:4] + '-' + datestr[4:6] + '-' + datestr[6:]
      return None
    elif d!=' ' and d!='-':
      return None
    datepos = datepos + 1

path = '.'
if len(sys.argv) == 2:
    path = sys.argv[1]

mainpage = open(path + '/daca2.html', 'wt')
mainpage.write('<html>\n')
mainpage.write('<head><title>DACA2</title></head>\n')
mainpage.write('<body>\n')
mainpage.write('<h1>DACA2</h1>\n')
mainpage.write('<p>Results when running latest Cppcheck on Debian.</p>\n')

lastupdate = None
recent = []

daca2 = os.path.expanduser('~/daca2/')
for lib in range(2):
  for a in "0123456789abcdefghijklmnopqrstuvwxyz":
    if lib == 1:
      a = "lib" + a
    if os.path.isfile(daca2 + a + '/results.txt'):
      f = open(daca2 + a + '/results.txt', 'rt')
      data = f.read()
      f.close()

      datestr = readdate(data)
      if datestr:
        if not lastupdate or datestr > lastupdate:
          lastupdate = datestr
          recent = []
        if datestr == lastupdate:
          recent.append(a)

      mainpage.write('<a href="daca2-'+a+'.html">'+a+'</a><br>\n')

      data = data.replace('&', '&nbsp;')
      data = data.replace('<', '&lt;')
      data = data.replace('>', '&gt;')
      data = data.replace('\'', '&apos;')
      data = data.replace('"', '&quot;')
      data = data.replace('\n', '\n')

      f = open(path+'/daca2-'+a+'.html', 'wt')
      f.write('<html>\n')
      f.write('<head><title>DACA2 - ' + a + '</title></head>\n')
      f.write('<body>\n')
      f.write('<h1>DACA2 - ' + a + '</h1>')
      f.write('<pre>\n' + data + '</pre>\n')
      f.write('</body>\n')
      f.write('</html>\n')
      f.close()

if lastupdate:
  mainpage.write('<p>Last update: ' + lastupdate + '</p>')
  allrecent = ''
  for r in recent:
    allrecent = allrecent + ' <a href="daca2-'+r+'.html">' + r + '</a>'
  mainpage.write('<p>Most recently updated: ' + allrecent + '</p>')

mainpage.write('</body>\n')
mainpage.write('</html>\n')
