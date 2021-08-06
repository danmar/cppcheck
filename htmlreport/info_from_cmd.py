import subprocess
import os
import datetime
import locale


def get_results(line, path, file, blame_options):
    head, tail = os.path.split(file)
    if head != "":
        path = head
    try:
        os.chdir(path)
    except:
        return ""
    try:
        result = subprocess.check_output(f'git blame -L {str(line)}{" -w" if "-w" in blame_options else ""}{" -M" if "-M" in blame_options else ""} --porcelain -- {file}')
        result = result.decode(locale.getpreferredencoding())
    except:
        return ""

    return result


def get_author(text):
    author = text[text.find('author') + 7:text.find('author-mail')-1]
    if text.startswith('fatal') or author == '':        # if error occurs from cmd starting with fatal sets the author to 'unknown'
        author = 'Unknown'
    disallowed_characters = '\<>'

    for character in disallowed_characters:
        author = author.replace(character, "")

    return author


def get_author_mail(text):
    author_mail = text[text.find('author-mail') + 13:text.find('author-time')-2]
    if text.startswith('fatal') or author_mail == '':   # if error occurs from cmd starting with fatal sets the author mail to '---'
        author_mail = '---'

    disallowed_characters = '\<>'

    for character in disallowed_characters:
        author_mail = author_mail.replace(character, "")

    return author_mail


def get_time(text):
    if text.startswith('fatal') or text == '':      # if error occurs from cmd starting with fatal sets the time to '---'
        DD_MM_YYYY = '---'
    else:
        commit_time = text[text.find('author-time') + 12:text.find('author-tz') - 1]
        datetime_object = datetime.date.fromtimestamp(int(commit_time))
        year = datetime_object.strftime("%Y")
        month = datetime_object.strftime("%m")
        day = datetime_object.strftime("%d")
        DD_MM_YYYY = f'{day}/{month}/{year}'
    return DD_MM_YYYY
