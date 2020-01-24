#!/usr/bin/env python

from __future__ import unicode_literals

import io
import sys
import optparse
import os
import operator

from collections import Counter
from pygments import highlight
from pygments.lexers import guess_lexer, guess_lexer_for_filename
from pygments.formatters import HtmlFormatter  # pylint: disable=no-name-in-module
from pygments.util import ClassNotFound
from xml.sax import parse as xml_parse
from xml.sax import SAXParseException as XmlParseException
from xml.sax.handler import ContentHandler as XmlContentHandler
from xml.sax.saxutils import escape
"""
Turns a cppcheck xml file into a browsable html report along
with syntax highlighted source code.
"""

STYLE_FILE = """
body {
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, "Noto Sans", sans-serif;
    font-size: 13px;
    line-height: 1.5;
    margin: 0;
    width: auto;
}

h1 {
    margin: 10px;
}

.header {
    border-bottom: thin solid #aaa;
}

.footer {
    border-top: thin solid #aaa;
    font-size: 90%;
    margin-top: 5px;
}

.footer ul {
    list-style-type: none;
    padding-left: 0;
}

.footer > p {
    margin: 4px;
}

.wrapper {
    display: -webkit-box;
    display: -ms-flexbox;
    display: flex;
    -webkit-box-pack: justify;
    -ms-flex-pack: justify;
    justify-content: space-between;
}

#menu,
#menu_index {
    text-align: left;
    width: 350px;
    height: 90vh;
    min-height: 200px;
    overflow: auto;
    position: -webkit-sticky;
    position: sticky;
    top: 0;
    padding: 0 15px 15px 15px;
}

#menu > a {
    display: block;
    margin-left: 10px;
    font-size: 12px;
    z-index: 1;
}

#content,
#content_index {
    background-color: #fff;
    -webkit-box-sizing: content-box;
    -moz-box-sizing: content-box;
    box-sizing: content-box;
    padding: 0 15px 15px 15px;
    width: calc(100% - 350px);
    height: 100%;
    overflow-x: auto;
}

#filename {
    margin-left: 10px;
    font-size: 12px;
    z-index: 1;
}

.error {
    background-color: #ffb7b7;
}

.error2 {
    background-color: #faa;
    display: inline-block;
    margin-left: 4px;
}

.inconclusive {
    background-color: #b6b6b4;
}

.inconclusive2 {
    background-color: #b6b6b4;
    display: inline-block;
    margin-left: 4px;
}

.verbose {
    display: inline-block;
    vertical-align: top;
    cursor: help;
}

.verbose .content {
    display: none;
    position: absolute;
    padding: 10px;
    margin: 4px;
    max-width: 40%;
    white-space: pre-wrap;
    border: 1px solid #000;
    background-color: #ffffcc;
    cursor: auto;
}

.highlight .hll {
    padding: 1px;
}

.highlighttable {
    background-color: #fff;
    z-index: 10;
    position: relative;
    margin: -10px;
}

.linenos {
    border-right: thin solid #aaa;
    color: #d3d3d3;
    padding-right: 6px;
}

.d-none {
    display: none;
}
"""

HTML_HEAD = """
<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <title>Cppcheck - HTML report - %s</title>
    <link rel="stylesheet" href="style.css">
    <style>
%s
    </style>
    <script>
      function getStyle(el, styleProp) {
        var y;

        if (el.currentStyle) {
          y = el.currentStyle[styleProp];
        } else if (window.getComputedStyle) {
          y = document.defaultView.getComputedStyle(el, null).getPropertyValue(styleProp);
        }

        return y;
      }

      function toggle() {
        var el = this.expandable_content;
        var mark = this.expandable_marker;

        if (el.style.display === "block") {
          el.style.display = "none";
          mark.textContent = "[+]";
        } else {
          el.style.display = "block";
          mark.textContent = "[-]";
        }
      }

      function initExpandables() {
        var elements = document.querySelectorAll(".expandable");

        for (var i = 0, len = elements.length; i < len; i++) {
          var el = elements[i];
          var clickable = el.querySelector("span");
          var marker = clickable.querySelector(".marker");
          var content = el.querySelector(".content");
          var width = clickable.clientWidth - parseInt(getStyle(content, "padding-left")) - parseInt(getStyle(content, "padding-right"));
          content.style.width = width + "px";
          clickable.expandable_content = content;
          clickable.expandable_marker = marker;
          clickable.addEventListener("click", toggle);
        }
      }

      function toggleDisplay(id) {
        var elements = document.querySelectorAll("." + id);

        for (var i = 0, len = elements.length; i < len; i++) {
          elements[i].classList.toggle("d-none");
        }
      }

      function toggleAll() {
        var elements = document.querySelectorAll("input");

        // starting from 1 since 0 is the "toggle all" input
        for (var i = 1, len = elements.length; i < len; i++) {
          var el = elements[i];

          if (el.checked) {
            el.checked = false;
          } else {
            el.checked = true;
          }

          toggleDisplay(el.id);
        }
      }
      window.addEventListener("load", initExpandables);
    </script>
  </head>
  <body>
    <div id="header" class="header">
      <h1>Cppcheck report - %s: %s</h1>
    </div>
    <div class="wrapper">
      <div id="menu">
       <p id="filename"><a href="index.html">Defects:</a> %s</p>
"""

HTML_HEAD_END = """
    </div>
    <div id="content">
"""

HTML_FOOTER = """
      </div> <!-- /.wrapper -->
    </div>
    <div id="footer" class="footer">
      <p>
        Cppcheck %s - a tool for static C/C++ code analysis<br>
        <br>
        Internet: <a href="http://cppcheck.net">http://cppcheck.net</a><br>
        IRC: <a href="irc://irc.freenode.net/cppcheck">irc://irc.freenode.net/cppcheck</a><br>
      </p>
    </div>
  </body>
</html>
"""

HTML_ERROR = "<span class=\"error2\">&lt;--- %s</span>\n"
HTML_INCONCLUSIVE = "<span class=\"inconclusive2\">&lt;--- %s</span>\n"

HTML_EXPANDABLE_ERROR = "<div class=\"verbose expandable\"><span class=\"error2\">&lt;--- %s <span class=\"marker\">[+]</span></span><div class=\"content\">%s</div></div>\n"""
HTML_EXPANDABLE_INCONCLUSIVE = "<div class=\"verbose expandable\"><span class=\"inconclusive2\">&lt;--- %s <span class=\"marker\">[+]</span></span><div class=\"content\">%s</div></div>\n"""

# escape() and unescape() takes care of &, < and >.
html_escape_table = {
    '"': "&quot;",
    "'": "&apos;"
}
html_unescape_table = {v: k for k, v in html_escape_table.items()}


def html_escape(text):
    return escape(text, html_escape_table)


class AnnotateCodeFormatter(HtmlFormatter):
    errors = []

    def wrap(self, source, outfile):
        line_no = 1
        for i, t in HtmlFormatter.wrap(self, source, outfile):
            # If this is a source code line we want to add a span tag at the
            # end.
            if i == 1:
                for error in self.errors:
                    if error['line'] == line_no:
                        try:
                            if error['inconclusive'] == 'true':
                                # only print verbose msg if it really differs
                                # from actual message
                                if error.get('verbose') and (error['verbose'] != error['msg']):
                                    index = t.rfind('\n')
                                    t = t[:index] + HTML_EXPANDABLE_INCONCLUSIVE % (error['msg'], html_escape(error['verbose'].replace("\\012", '\n'))) + t[index + 1:]
                                else:
                                    t = t.replace('\n', HTML_INCONCLUSIVE % error['msg'])
                        except KeyError:
                            if error.get('verbose') and (error['verbose'] != error['msg']):
                                index = t.rfind('\n')
                                t = t[:index] + HTML_EXPANDABLE_ERROR % (error['msg'], html_escape(error['verbose'].replace("\\012", '\n'))) + t[index + 1:]
                            else:
                                t = t.replace('\n', HTML_ERROR % error['msg'])

                line_no = line_no + 1
            yield i, t


class CppCheckHandler(XmlContentHandler):

    """Parses the cppcheck xml file and produces a list of all its errors."""

    def __init__(self):
        XmlContentHandler.__init__(self)
        self.errors = []
        self.version = '1'
        self.versionCppcheck = ''

    def startElement(self, name, attributes):
        if name == 'results':
            self.version = attributes.get('version', self.version)

        if self.version == '1':
            self.handleVersion1(name, attributes)
        else:
            self.handleVersion2(name, attributes)

    def handleVersion1(self, name, attributes):
        if name != 'error':
            return

        self.errors.append({
            'file': attributes.get('file', ''),
            'line': int(attributes.get('line', 0)),
            'locations': [{
                'file': attributes.get('file', ''),
                'line': int(attributes.get('line', 0)),
            }],
            'id': attributes['id'],
            'severity': attributes['severity'],
            'msg': attributes['msg']
        })

    def handleVersion2(self, name, attributes):
        if name == 'cppcheck':
            self.versionCppcheck = attributes['version']
        if name == 'error':
            error = {
                'locations': [],
                'file': '',
                'line': 0,
                'id': attributes['id'],
                'severity': attributes['severity'],
                'msg': attributes['msg'],
                'verbose': attributes.get('verbose')
            }

            if 'inconclusive' in attributes:
                error['inconclusive'] = attributes['inconclusive']
            if 'cwe' in attributes:
                error['cwe'] = attributes['cwe']

            self.errors.append(error)
        elif name == 'location':
            assert self.errors
            error = self.errors[-1]
            locations = error['locations']
            file = attributes['file']
            line = int(attributes['line'])
            if not locations:
                error['file'] = file
                error['line'] = line
            locations.append({
                'file': file,
                'line': line,
                'info': attributes.get('info')
            })

if __name__ == '__main__':
    # Configure all the options this little utility is using.
    parser = optparse.OptionParser()
    parser.add_option('--title', dest='title',
                      help='The title of the project.',
                      default='[project name]')
    parser.add_option('--file', dest='file',
                      help='The cppcheck xml output file to read defects '
                           'from. Default is reading from stdin.')
    parser.add_option('--report-dir', dest='report_dir',
                      help='The directory where the HTML report content is '
                           'written.')
    parser.add_option('--source-dir', dest='source_dir',
                      help='Base directory where source code files can be '
                           'found.')
    parser.add_option('--source-encoding', dest='source_encoding',
                      help='Encoding of source code.', default='utf-8')

    # Parse options and make sure that we have an output directory set.
    options, args = parser.parse_args()

    try:
        sys.argv[1]
    except IndexError:  # no arguments give, print --help
        parser.print_help()
        quit()

    if not options.report_dir:
        parser.error('No report directory set.')

    # Get the directory where source code files are located.
    source_dir = os.getcwd()
    if options.source_dir:
        source_dir = options.source_dir

    # Get the stream that we read cppcheck errors from.
    input_file = sys.stdin
    if options.file:
        if not os.path.exists(options.file):
            parser.error('cppcheck xml file: %s not found.' % options.file)
        input_file = io.open(options.file, 'r')
    else:
        parser.error('No cppcheck xml file specified. (--file=)')

    # Parse the xml file and produce a simple list of errors.
    print('Parsing xml report.')
    try:
        contentHandler = CppCheckHandler()
        xml_parse(input_file, contentHandler)
    except XmlParseException as msg:
        print('Failed to parse cppcheck xml file: %s' % msg)
        sys.exit(1)

    # We have a list of errors. But now we want to group them on
    # each source code file. Lets create a files dictionary that
    # will contain a list of all the errors in that file. For each
    # file we will also generate a HTML filename to use.
    files = {}
    file_no = 0
    for error in contentHandler.errors:
        filename = error['file']
        if filename not in files.keys():
            files[filename] = {
                'errors': [], 'htmlfile': str(file_no) + '.html'}
            file_no = file_no + 1
        files[filename]['errors'].append(error)

    # Make sure that the report directory is created if it doesn't exist.
    print('Creating %s directory' % options.report_dir)
    if not os.path.exists(options.report_dir):
        os.makedirs(options.report_dir)

    # Generate a HTML file with syntax highlighted source code for each
    # file that contains one or more errors.
    print('Processing errors')

    decode_errors = []
    for filename, data in sorted(files.items()):
        htmlfile = data['htmlfile']
        errors = []

        for error in data['errors']:
            for location in error['locations']:
                if filename == location['file']:
                    newError = dict(error)

                    del newError['locations']
                    newError['line'] = location['line']
                    if location.get('info'):
                        newError['msg'] = location['info']
                        newError['severity'] = 'information'
                        del newError['verbose']

                    errors.append(newError)

        lines = []
        for error in errors:
            lines.append(error['line'])

        if filename == '':
            continue

        source_filename = os.path.join(source_dir, filename)
        try:
            with io.open(source_filename, 'r', encoding=options.source_encoding) as input_file:
                content = input_file.read()
        except IOError:
            if (error['id'] == 'unmatchedSuppression'):
                continue  # file not found, bail out
            else:
                sys.stderr.write("ERROR: Source file '%s' not found.\n" %
                                 source_filename)
            continue
        except UnicodeDecodeError:
            sys.stderr.write("WARNING: Unicode decode error in '%s'.\n" %
                             source_filename)
            decode_errors.append(source_filename[2:])  # "[2:]" gets rid of "./" at beginning
            continue

        htmlFormatter = AnnotateCodeFormatter(linenos=True,
                                              style='colorful',
                                              hl_lines=lines,
                                              lineanchors='line',
                                              encoding=options.source_encoding)
        htmlFormatter.errors = errors

        with io.open(os.path.join(options.report_dir, htmlfile), 'w', encoding='utf-8') as output_file:
            output_file.write(HTML_HEAD %
                              (options.title,
                               htmlFormatter.get_style_defs('.highlight'),
                               options.title,
                               filename,
                               filename.split('/')[-1]))

            for error in sorted(errors, key=lambda k: k['line']):
                output_file.write("<a href=\"%s#line-%d\"> %s %s</a>" % (data['htmlfile'], error['line'], error['id'],   error['line']))

            output_file.write(HTML_HEAD_END)
            try:
                lexer = guess_lexer_for_filename(source_filename, '', stripnl=False)
            except ClassNotFound:
                try:
                    lexer = guess_lexer(content, stripnl=False)
                except ClassNotFound:
                    sys.stderr.write("ERROR: Couldn't determine lexer for the file' " + source_filename + " '. Won't be able to syntax highlight this file.")
                    output_file.write("\n <tr><td colspan=\"5\"> Could not generate content because pygments failed to determine the code type.</td></tr>")
                    output_file.write("\n <tr><td colspan=\"5\"> Sorry about this.</td></tr>")
                    continue

            if options.source_encoding:
                lexer.encoding = options.source_encoding

            output_file.write(
                highlight(content, lexer, htmlFormatter).decode(
                    options.source_encoding))

            output_file.write(HTML_FOOTER % contentHandler.versionCppcheck)

        print('  ' + filename)

    # Generate a master index.html file that will contain a list of
    # all the errors created.
    print('Creating index.html')

    with io.open(os.path.join(options.report_dir, 'index.html'),
                 'w') as output_file:

        stats_count = 0
        stats = []
        for filename, data in sorted(files.items()):
            for error in data['errors']:
                stats.append(error['id'])  # get the stats
                stats_count += 1

        counter = Counter(stats)

        stat_html = []
        # the following lines sort the stat primary by value (occurrences),
        # but if two IDs occur equally often, then we sort them alphabetically by warning ID
        try:
            cnt_max = counter.most_common()[0][1]
        except IndexError:
            cnt_max = 0

        try:
            cnt_min = counter.most_common()[-1][1]
        except IndexError:
            cnt_min = 0

        stat_fmt = "\n            <tr><td><input type=\"checkbox\" onclick=\"toggleDisplay(this.id)\" id=\"{}\" name=\"{}\" checked></td><td>{}</td><td>{}</td></tr>"
        for occurrences in reversed(range(cnt_min, cnt_max + 1)):
            for _id in [k for k, v in sorted(counter.items()) if v == occurrences]:
                stat_html.append(stat_fmt.format(_id, _id, dict(counter.most_common())[_id], _id))

        output_file.write(HTML_HEAD.replace('id="menu"', 'id="menu_index"', 1).replace("Defects:", "Defect summary;", 1) % (options.title, '', options.title, '', ''))
        output_file.write('\n       <label><input type="checkbox" onclick="toggleAll()" checked> Toggle all</label>')
        output_file.write('\n       <table>')
        output_file.write('\n           <tr><th>Show</th><th>#</th><th>Defect ID</th></tr>')
        output_file.write(''.join(stat_html))
        output_file.write('\n           <tr><td></td><td>' + str(stats_count) + '</td><td>total</td></tr>')
        output_file.write('\n       </table>')
        output_file.write('\n       <p><a href="stats.html">Statistics</a></p>')
        output_file.write(HTML_HEAD_END.replace("content", "content_index", 1))

        output_file.write('\n       <table>')
        output_file.write('\n       <tr><th>Line</th><th>Id</th><th>CWE</th><th>Severity</th><th>Message</th></tr>')
        for filename, data in sorted(files.items()):
            if filename in decode_errors:  # don't print a link but a note
                output_file.write("\n       <tr><td colspan=\"5\">%s</td></tr>" % (filename))
                output_file.write("\n       <tr><td colspan=\"5\"> Could not generated due to UnicodeDecodeError</td></tr>")
            else:
                if filename.endswith('*'):  # assume unmatched suppression
                    output_file.write(
                        "\n       <tr><td colspan=\"5\">%s</td></tr>" %
                        (filename))
                else:
                    output_file.write(
                        "\n       <tr><td colspan=\"5\"><a href=\"%s\">%s</a></td></tr>" %
                        (data['htmlfile'], filename))

                for error in sorted(data['errors'], key=lambda k: k['line']):
                    error_class = ''
                    try:
                        if error['inconclusive'] == 'true':
                            error_class = 'class="inconclusive"'
                            error['severity'] += ", inconcl."
                    except KeyError:
                        pass

                    try:
                        if error['cwe']:
                            cwe_url = "<a href=\"https://cwe.mitre.org/data/definitions/" + error['cwe'] + ".html\">" + error['cwe'] + "</a>"
                    except KeyError:
                        cwe_url = ""

                    if error['severity'] == 'error':
                        error_class = 'class="error"'
                    if error['id'] == 'missingInclude':
                        output_file.write(
                            '\n         <tr class="%s"><td></td><td>%s</td><td></td><td>%s</td><td>%s</td></tr>' %
                            (error['id'], error['id'], error['severity'], html_escape(error['msg'])))
                    elif (error['id'] == 'unmatchedSuppression') and filename.endswith('*'):
                        output_file.write(
                            '\n         <tr class="%s"><td></td><td>%s</td><td></td><td>%s</td><td %s>%s</td></tr>' %
                            (error['id'], error['id'], error['severity'], error_class,
                             html_escape(error['msg'])))
                    else:
                        output_file.write(
                            '\n       <tr class="%s"><td><a href="%s#line-%d">%d</a></td><td>%s</td><td>%s</td><td>%s</td><td %s>%s</td></tr>' %
                            (error['id'], data['htmlfile'], error['line'], error['line'],
                             error['id'], cwe_url, error['severity'], error_class,
                             html_escape(error['msg'])))

        output_file.write('\n       </table>')
        output_file.write(HTML_FOOTER % contentHandler.versionCppcheck)

    if (decode_errors):
        sys.stderr.write("\nGenerating html failed for the following files: " + ' '.join(decode_errors))
        sys.stderr.write("\nConsider changing source-encoding (for example: \"htmlreport ... --source-encoding=\"iso8859-1\"\"\n")

    print('Creating style.css file')
    with io.open(os.path.join(options.report_dir, 'style.css'),
                 'w') as css_file:
        css_file.write(STYLE_FILE)

    print("Creating stats.html (statistics)\n")
    stats_countlist = {}

    for filename, data in sorted(files.items()):
        if (filename == ''):
            continue
        stats_tmplist = []
        for error in sorted(data['errors'], key=lambda k: k['line']):
            stats_tmplist.append(error['severity'])

        stats_countlist[filename] = dict(Counter(stats_tmplist))

    # get top ten for each severity
    SEVERITIES = "error", "warning", "portability", "performance", "style", "unusedFunction", "information", "missingInclude", "internal"

    with io.open(os.path.join(options.report_dir, 'stats.html'), 'w') as stats_file:

        stats_file.write(HTML_HEAD.replace('id="menu"', 'id="menu_index"', 1).replace("Defects:", "Back to summary", 1) % (options.title, '', options.title, 'Statistics', ''))
        stats_file.write(HTML_HEAD_END.replace("content", "content_index", 1))

        for sev in SEVERITIES:
            _sum = 0
            stats_templist = {}

            # if the we have an style warning but we are checking for
            # portability, we have to skip it to prevent KeyError
            try:
                for filename in stats_countlist:
                    try:  # also bail out if we have a file with no sev-results
                        _sum += stats_countlist[filename][sev]
                        stats_templist[filename] = (int)(stats_countlist[filename][sev])  # file : amount,
                    except KeyError:
                        continue
                # don't print "0 style" etc, if no style warnings were found
                if (_sum == 0):
                    continue
            except KeyError:
                continue
            stats_file.write("<p>Top 10 files for " + sev + " severity, total findings: " + str(_sum) + "<br>\n")

            # sort, so that the file with the most severities per type is first
            stats_list_sorted = sorted(stats_templist.items(), key=operator.itemgetter(1, 0), reverse=True)
            it = 0
            LENGTH = 0

            for i in stats_list_sorted:  # printing loop
                # for aesthetics: if it's the first iteration of the loop, get
                # the max length of the number string
                if (it == 0):
                    LENGTH = len(str(i[1]))  # <- length of longest number, now get the difference and try to  make other numbers align to it

                stats_file.write("&#160;" * 3 + str(i[1]) + "&#160;" * (1 + LENGTH - len(str(i[1]))) + "<a href=\"" + files[i[0]]['htmlfile'] + "\">  " + i[0] + "</a><br>\n")
                it += 1
                if (it == 10):  # print only the top 10
                    break
            stats_file.write("</p>\n")

        stats_file.write(HTML_FOOTER % contentHandler.versionCppcheck)

    print("\nOpen '" + options.report_dir + "/index.html' to see the results.")
