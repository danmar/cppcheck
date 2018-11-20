
import requests
import subprocess
import sys

response = requests.get('https://api.github.com/repos/danmar/cppcheck/pulls/' + sys.argv[1])
if response.status_code == 200:
    j = response.json()
    login = j['user']['login']
    title = j['title']
    body  = j['body']
    branch = j['head']['ref']
    sha = j['head']['sha']

    subprocess.call('git checkout -b {}-{} master'.format(login, branch).split())
    subprocess.call('git pull https://github.com/{}/cppcheck.git {}'.format(login, branch).split())

    p = subprocess.Popen(['git', 'show', '--format=%an <%ae>', sha], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    author = comm[0].decode(encoding='utf-8', errors='ignore').split('\n')[0]
    if login == 'pfultz2':
        author = 'Paul Fultz II ' + author[author.find('<'):]

    subprocess.call(['./runastyle'])
    subprocess.call('git commit -a -m astyle'.split())

    subprocess.call('git checkout master'.split())
    subprocess.call('git merge --squash {}-{}'.format(login, branch).split())
    subprocess.call(['git', 'commit', '-a', '--author='+author, '-m', title + '\n\n' + body])
    subprocess.call('git branch -D {}-{}'.format(login, branch).split())

    p = subprocess.Popen('git show --format=format:%h'.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    comm = p.communicate()
    stdout = comm[0]
    if stdout.find('\n') > 0:
        stdout = stdout[:stdout.find('\n')]
    print('\nMessage: I merged this with ' + stdout)



