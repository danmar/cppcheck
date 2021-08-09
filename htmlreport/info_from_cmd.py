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
        result = subprocess.check_output('git blame -L %d %s %s --porcelain -- %s' % (
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


def tr_str(tr_class, td_th, datahtml, line, id, cwe, severity, error_class, message, add_author=False, author=None, author_mail=None, date=None):
    ret = ''
    if datahtml:
        ret += '<%s><a href="%s#line-%d">%d</a></%s>' % (td_th, datahtml, line, line, td_th)
        for item in (id, cwe, severity):
            ret += '<%s>%s</%s>' % (td_th, item, td_th)
    else:
        for item in (line, id, cwe, severity):
            ret += '<%s>%s</%s>' % (td_th, item, td_th)

    if error_class:
        ret += '<%s %s>%s</%s>' % (td_th, error_class, message, td_th)
    else:
        ret += '<%s>%s</%s>' % (td_th, message, td_th)

    if add_author:
        for item in (author, author_mail, date):
            ret += '<%s>%s</%s>' % (td_th, item, td_th)
    if tr_class:
        tr_attributes = ' class="%s"' % tr_class
    else:
        tr_attributes = ''
    return '<tr%s>%s</tr>' % (tr_attributes, ret)
