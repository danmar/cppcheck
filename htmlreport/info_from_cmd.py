import subprocess
import os
import datetime
import locale


def git_blame(line, path, file, blame_options):
    git_blame_dict = {
        'author': 'Unknown',
        'author_mail': '---',
        'date': '---'
    }
    head, tail = os.path.split(file)
    if head != "":
        path = head

    try:
        os.chdir(path)
    except:
        return git_blame_dict

    try:
        result = subprocess.check_output('git blame -L %d%s%s --porcelain -- %s' % (line, " -w" if "-w" in blame_options else "", " -M" if "-M" in blame_options else "", file))
        result = result.decode(locale.getpreferredencoding())
    except:
        return git_blame_dict

    if result.startswith('fatal'):
        return git_blame_dict

    author = result[result.find('author') + 6:result.find('author-mail') - 1]
    author_mail = result[result.find('author-mail') + 11:result.find('author-time') - 1]
    commit_time = result[result.find('author-time') + 11:result.find('author-tz') - 1]

    disallowed_characters = '\ <>'

    for character in disallowed_characters:
        author = author.replace(character, "")

    for character in disallowed_characters:
        author_mail = author_mail.replace(character, "")

    datetime_object = datetime.date.fromtimestamp(int(commit_time))
    year = datetime_object.strftime("%Y")
    month = datetime_object.strftime("%m")
    day = datetime_object.strftime("%d")

    git_blame_dict['author'] = author
    git_blame_dict['author_mail'] = author_mail
    git_blame_dict['date'] = '%s/%s/%s' % (day, month, year)

    return git_blame_dict
