#!/usr/bin/env python

import csv
import optparse
import sys
import xml.etree.ElementTree as ET


class CppCheckHandler:
    def __init__(self, file):
        self.file = file
        self.data = self.read_file()
        self.tree = ET.parse(self.file)
        self.root = self.tree.getroot()
        self.version = self.root.attrib['version']
        self.errors = self.get_errors()

    def read_file(self):
        with open(self.file) as f:
            data = f.read()
        return data

    def get_errors(self):
        errors = []
        for error in self.root.iter('error'):
            for location in error:
                try:
                    new_error = {
                        'file': location.attrib['file'],
                        'line': location.attrib['line'],
                        'id': error.attrib['id'],
                        'severity': error.attrib['severity'],
                        'msg': error.attrib['msg'],
                        'verbose': error.attrib['verbose'],
                        'author': location.attrib['author'],
                        'author_mail': location.attrib.get('author_mail'),
                        'date': location.attrib.get('date')
                    }
                    errors.append(new_error)
                except:
                    pass

        return errors


if __name__ == '__main__':
    # Configure all the options this little utility is using.
    parser = optparse.OptionParser()
    parser.add_option('--input-file', '--if', dest='file', action="append",
                      help='The cppcheck xml output file to read defects '
                           'from')
    parser.add_option('--result-file', '--rf', dest='result_file', action="append",
                      help='The cppcheck xml output file to transform '
                           'into csv file')

    # Parse options and make sure that we have an output directory set.
    options, args = parser.parse_args()

    try:
        sys.argv[1]
    except IndexError:  # no arguments give, print --help
        parser.print_help()
        quit()

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
    result_file = options.result_file[0]

    contentHandler = CppCheckHandler(data)
    errors = contentHandler.errors

    if result_file.endswith('.csv'):
        with open(result_file, 'w', newline='') as f:
            writer = csv.writer(f, dialect='excel', delimiter=';')
            writer.writerow(errors[0].keys())
            for error in errors:
                writer.writerow(error.values())

    print(f'\nOpen {result_file} to see the results')
