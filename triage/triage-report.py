
import glob
import sys
import re

if len(sys.argv) != 3:
    print('usage: triage.py project resultsfile.txt')
    sys.exit(1)

project = sys.argv[1]
resultfile = sys.argv[2]

f = open(project + '/true-positives.txt', 'rt')
truepositives = f.read()
f.close()

f = open(project + '/false-positives.txt', 'rt')
falsepositives = f.read()
f.close()

fin = open(resultfile, 'rt')
results = fin.read()
fin.close()

out = {}
out['untriaged'] = ''
out['fn'] = ''
out['fp'] = ''
out['tp'] = ''

numberOfFalsePositives = 0
numberOfTruePositives = 0
numberOfFalseNegatives = 0

for result in results.split('\n'):
    result = result.strip()

    res = re.match('\\[(' + project + '.+):([0-9]+)\\]:\s+[(][a-z]+[)] (.+)', result)
    if res is None:
        continue

    filename = res.group(1)
    linenr = res.group(2)
    message = res.group(3)
    css = 'untriaged'
    classification = 'Untriaged'
    if result in truepositives:
        css = 'tp'
        classification = 'True Positive'
        numberOfTruePositives += 1
    elif result in falsepositives:
        css = 'fp'
        classification = 'False Positive'
        numberOfFalsePositives += 1
    href = None

    html = '  <tr>'
    html += '<td class=' + css + '>' + filename + '</td>'
    html += '<td class=' + css + '>' + linenr + '</td>'
    html += '<td class=' + css + '>' + message + '</td>'
    if project == 'linux-3.11':
        href = 'http://github.com/torvalds/linux/blob/v3.11' + filename[filename.find('/'):] + '#L' + linenr
    if href:
        html += '<td class=' + css + '><a href="' + href + '">' + classification + '</a></td>'
    else:
        html += '<td class=' + css + '>' + classification + '</td>'
    html += '</tr>\n'

    out[css] += html

f = open(project + '/true-positives.txt', 'rt')
for line in f.readlines():
    line = line.strip()
    if line.find('] -> [') > 0 or line.find('(error)') < 0:
        continue

    res = re.match('\\[(' + project + '.+):([0-9]+)\\]:\s+[(][a-z]+[)] (.+)', line)
    if res is None:
        continue

    if line in results:
        continue

    numberOfFalseNegatives += 1

    filename = res.group(1)
    linenr = res.group(2)
    message = res.group(3)
    classification = 'False Negative'
    css = 'fn'

    html = '  <tr>'
    html += '<td class=' + css + '>' + filename + '</td>'
    html += '<td class=' + css + '>' + linenr + '</td>'
    html += '<td class=' + css + '>' + message + '</td>'
    html += '<td class=' + css + '>' + classification + '</td>'
    html += '</tr>\n'

    out[css] += html

f.close()

project2 = ''
if project.find('-') > 0:
    project2 = project[:project.find('-')]
else:
    project2 = project

fout = open('report.html', 'wt')
fout.write('<html><head><title>Cppcheck results for ' + project + '</title><link rel="stylesheet" type="text/css" href="theme1.css"></head><body>\n')
fout.write('<h1>Cppcheck results for ' + project + '</h1>\n')
fout.write('<p>Number of false negatives: ' + str(numberOfFalseNegatives) + '</p>\n')
fout.write('<p>Number of true positives: ' + str(numberOfTruePositives) + '</p>\n')
fout.write('<p>Number of false positives: ' + str(numberOfFalsePositives) + '</p>\n')
fout.write('<table border="0">\n')
fout.write('<tr><th>Filename</th><th>Line</th><th>Message</th><th>Classification</th></tr>\n')
fout.write(out['fn'])
fout.write(out['tp'])
fout.write(out['fp'])
fout.write(out['untriaged'])

fout.write('</table></body></html>')
fout.close()
