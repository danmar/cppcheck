import subprocess
import os
import datetime
import locale


def git_blame(line, path, file, blame_options):
    git_blame_dict = {}
    head, tail = os.path.split(file)
    if head != "":
        path = head

    try:
        os.chdir(path)
    except:
        return {'author': 'Unknown', 'author-mail': '---', 'author-time': '---'}

    try:
        result = subprocess.check_output('git blame -L %d%s%s --porcelain -- %s' % (
            line, " -w" if "-w" in blame_options else "", " -M" if "-M" in blame_options else "", file))
        result = result.decode(locale.getpreferredencoding())
    except:
        return {'author': 'Unknown', 'author-mail': '---', 'author-time': '---'}

    if result.startswith('fatal'):
        return {'author': 'Unknown', 'author-mail': '---', 'author-time': '---'}

    disallowed_characters = '<>'
    for line in result.split('\n')[1:]:
        space_pos = line.find(' ')
        if space_pos > 30:
            break
        key = line[:space_pos]
        val = line[space_pos + 1:]

        for character in disallowed_characters:
            val = val.replace(character, "")
        git_blame_dict[key] = val

    datetime_object = datetime.date.fromtimestamp(float(git_blame_dict['author-time']))
    year = datetime_object.strftime("%Y")
    month = datetime_object.strftime("%m")
    day = datetime_object.strftime("%d")

    git_blame_dict['author-time'] = '%s/%s/%s' % (day, month, year)

    return git_blame_dict
