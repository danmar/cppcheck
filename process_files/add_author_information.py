#!/usr/bin/env python

import optparse
import sys
import xml.etree.ElementTree as ET
import subprocess
import os
import locale
import datetime


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
        result = subprocess.check_output(
            f'git blame -L {str(line)}{" -w" if "-w" in blame_options else ""}{" -M" if "-M" in blame_options else ""} --porcelain -- {file}')
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
        git_blame_dict[key] = val.encode('UTF-8')
    datetime_object = datetime.date.fromtimestamp(float(git_blame_dict['author-time']))
    year = datetime_object.strftime("%Y")
    month = datetime_object.strftime("%m")
    day = datetime_object.strftime("%d")

    git_blame_dict['author-time'] = '%s/%s/%s' % (day, month, year)
    return git_blame_dict


if __name__ == '__main__':
    parser = optparse.OptionParser()
    parser.add_option('--input-file', '--if', dest='file', action="append",
                      help='The cppcheck xml output file to read defects '
                           'from')
    parser.add_option('--source-dir', '--sd', dest='source_dir',
                      help='Base directory where source code files can be '
                           'found.')
    parser.add_option('--blame-options', '--bo', dest='blame_options', action="append",
                      help='[-w, -M] blame options which you can use to get author and author mail  '
                           '-w --> not including white spaces and returns original author of the line  '
                           '-M --> not including moving of lines and returns original author of the line')
    parser.add_option('--result-file', '--rf', dest='result_file', action="append",
                      help='The cppcheck xml output file to transform '
                           'into xml file with author, mail and date')

    # Parse options and make sure that we have an output directory set.
    options, args = parser.parse_args()

    try:
        sys.argv[3]
    except IndexError:  # no arguments give, print --help
        parser.print_help()
        quit()

    # Get the directory where source code files are located.
    cwd = os.getcwd()
    source_dir = os.getcwd()
    if options.source_dir:
        source_dir = options.source_dir

    blame_options = ""
    if options.blame_options:
        blame_options = options.blame_options

    if options.file is None or "":
        print('No file selected')
        print('Try \'--help\'')
        sys.exit(1)

    if options.result_file is None or "":
        print('No result file selected')
        print('Try \'--help\'')
        sys.exit(1)

    if options.file == options.result_file:
        print('Result file can\'t be the same as source file')
        sys.exit(1)

    data = options.file[0]
    with open(data, 'r') as f:
        first_line = f.readline()
    result_file = options.result_file[0]
    try:
        tree = ET.parse(data)
    except:
        print('File not found')
        sys.exit(1)

    if result_file.endswith('.xml'):
        result_file = result_file
    else:
        result_file += '.xml'

    open_file = open(result_file, "w", encoding='UTF-8')
    open_file.write(first_line)
    root = tree.getroot()
    locations = []
    if root.iter('location'):
        locations = []
        for location in root.iter('location'):
            locations.append(location)
    for elem in root:
        for i, subelem in enumerate(elem):
            if root.iter('location'):
                file = locations[i].attrib['file']
                line = locations[i].attrib['line']

            else:
                file = subelem[i].attrib['file']
                line = subelem[i].attrib['line']

            git_blame_dict = git_blame(line, source_dir, file, blame_options)
            locations[i].set('author', git_blame_dict['author'].decode('UTF-8'))
            locations[i].set('author_mail', git_blame_dict['author-mail'].decode('UTF-8'))
            locations[i].set('date', git_blame_dict['author-time'])

    open_file.write(ET.tostring(root).decode('UTF-8'))
    open_file.close()

    print(f'\nOpen {result_file} to see the results')
