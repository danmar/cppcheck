#!/usr/bin/env python
"""Test cppcheck-htmlreport."""

import os
import contextlib
import shutil
import subprocess
import sys
import tempfile

if sys.version_info < (2, 7):
    # For TestCase.assertIn().
    import unittest2 as unittest
else:
    import unittest

ROOT_DIR = os.path.split(os.path.abspath(os.path.dirname(__file__)))[0]
CPPCHECK_BIN = os.path.join(ROOT_DIR, 'cppcheck')

HTML_REPORT_BIN = os.path.join(os.path.abspath(os.path.dirname(__file__)),
                               'cppcheck-htmlreport')


class TestHTMLReport(unittest.TestCase):

    def testReportError(self):
        for xml_version in ['1', '2']:
            self.checkReportError(xml_version)

    def checkReportError(self, xml_version):
        with runCheck(
            os.path.join(ROOT_DIR, 'samples', 'memleak', 'bad.c'),
            xml_version=xml_version
        ) as (report, output_directory):
            self.assertIn('<html', report)

            self.assertIn('Memory leak:', report)
            self.assertIn('bad.c', report)

            detail_filename = os.path.join(output_directory, '0.html')
            self.assertTrue(
                os.path.exists(detail_filename))

            with open(detail_filename) as input_file:
                detail_contents = input_file.read()
                self.assertIn('<html', detail_contents)
                self.assertIn('Memory leak:', detail_contents)

    def testReportNoError(self):
        for xml_version in ['1', '2']:
            self.checkReportNoError(xml_version)

    def checkReportNoError(self, xml_version):
        with runCheck(
            os.path.join(ROOT_DIR, 'samples', 'memleak', 'good.c'),
            xml_version=xml_version
        ) as (report, output_directory):
            self.assertIn('<html', report)

            self.assertNotIn('Memory leak:', report)
            self.assertNotIn('good.c', report)

            self.assertFalse(
                os.path.exists(os.path.join(output_directory, '0.html')))

    def testMissingInclude(self):
        with runCheck(
            xml_filename=os.path.join(ROOT_DIR, 'htmlreport', 'example.xml'),
        ) as (report, output_directory):
            self.assertIn('<html', report)

            self.assertIn('Uninitialized variable:', report)
            self.assertIn('example.cc', report)

            self.assertTrue(
                os.path.exists(os.path.join(output_directory, '0.html')))


@contextlib.contextmanager
def runCheck(source_filename=None, xml_version='1', xml_filename=None):
    """Run cppcheck and cppcheck-htmlreport.

    Yield a tuple containing the resulting HTML report index and the directory
    path.

    """
    output_directory = tempfile.mkdtemp(dir='.')
    if xml_filename is None:
        assert source_filename
        xml_filename = os.path.join(output_directory, 'output.xml')

        with open(xml_filename, 'w') as output_file:
            subprocess.check_call(
                [CPPCHECK_BIN, '--xml', source_filename,
                 '--xml-version=' + xml_version],
                stderr=output_file)

    assert os.path.exists(xml_filename)

    subprocess.check_call(
        [HTML_REPORT_BIN,
         '--file=' + os.path.realpath(xml_filename),
         '--report-dir=' + os.path.realpath(output_directory)],
        cwd=os.path.join(ROOT_DIR, 'htmlreport'))

    with open(os.path.join(output_directory, 'index.html')) as index_file:
        index_contents = index_file.read()

    yield (index_contents, output_directory)

    shutil.rmtree(output_directory)


if __name__ == '__main__':
    unittest.main()
