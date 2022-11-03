#!/usr/bin/env python3

from __future__ import unicode_literals

from datetime import date
import io
import locale
import operator
import optparse
import os
import re
import sys
import subprocess

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
    height: 100%;
    margin: 0;
}

#wrapper {
    position: fixed;
    height: 100vh;
    width: 100vw;
    display: grid;
    grid-template-rows: fit-content(8rem) auto fit-content(8rem);
    grid-template-columns: fit-content(25%) 1fr;
    grid-template-areas:
      "header header"
      "menu content"
      "footer footer";
}

h1 {
    margin: 0 0 8px -2px;
    font-size: 175%;
}

.header {
    padding: 0 0 5px 15px;
    grid-area: header;
    border-bottom: thin solid #aaa;
}

.footer {
    grid-area: footer;
    border-top: thin solid #aaa;
    font-size: 85%;

}

.footer > p {
    margin: 4px;
}

#menu,
#menu_index {
    grid-area: menu;
    text-align: left;
    overflow: auto;
    padding: 0 23px 15px 15px;
    border-right: thin solid #aaa;
    min-width: 200px;
}

#menu > a {
    display: block;
    margin-left: 10px;
    font-size: 12px;
}

#content,
#content_index {
    grid-area: content;
    padding: 0px 5px 15px 15px;
    overflow: auto;
}

label {
    white-space: nowrap;
}

label.checkBtn.disabled {
    color: #606060;
    background: #e0e0e0;
    font-style: italic;
}

label.checkBtn, input[type="text"] {
    border: 1px solid grey;
    border-radius: 4px;
    box-shadow: 1px 1px inset;
    padding: 1px 5px;
}

label.checkBtn {
    white-space: nowrap;
    background: #ccddff;
}

label.unchecked {
    background: #eff8ff;
    box-shadow: 1px 1px 1px;
}

label.checkBtn:hover, label.unchecked:hover{
    box-shadow: 0 0 2px;
}

label.disabled:hover {
    box-shadow: 1px 1px inset;
}

label.checkBtn > input {
    display:none;
}

.summaryTable {
    width: 100%;
}

table.summaryTable td { padding: 0 5px 0 5px; }

.statHeader, .severityHeader {
    font-weight: bold;
}

.warning {
    background-color: #ffffa7;
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
    position: relative;
    margin: -10px;
}

.linenos {
    border-right: thin solid #aaa;
    color: #d3d3d3;
    padding-right: 6px;
}

.id-filtered, .severity-filtered, .file-filtered, .tool-filtered, .text-filtered {
    visibility: collapse;
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

      function toggleDisplay(cb) {
        var elements = document.querySelectorAll("." + cb.id);

        for (var i = 0, len = elements.length; i < len; i++) {
          elements[i].classList.toggle("id-filtered", !cb.checked);
        }

        updateFileRows();
      }

      function toggleSeverity(cb) {
        cb.parentElement.classList.toggle("unchecked", !cb.checked);
        var elements = document.querySelectorAll(".sev_" + cb.id);

        for (var i = 0, len = elements.length; i < len; i++) {
          elements[i].classList.toggle("severity-filtered", !cb.checked);
        }

        updateFileRows();
      }

      function toggleTool(cb) {
        cb.parentElement.classList.toggle("unchecked", !cb.checked);

        var elements;
        if (cb.id == "clang-tidy")
            elements = document.querySelectorAll("[class^=clang-tidy-]");
        else
            elements = document.querySelectorAll(".issue:not([class^=clang-tidy-])");

        for (var i = 0, len = elements.length; i < len; i++) {
          elements[i].classList.toggle("tool-filtered", !cb.checked);
        }

        updateFileRows();
      }

      function toggleAll() {
        var elements = document.querySelectorAll(".idToggle");

        // starting from 1 since 0 is the "toggle all" input
        for (var i = 1, len = elements.length; i < len; i++) {
          var changed = elements[i].checked != elements[0].checked;
          if (changed) {
            elements[i].checked = elements[0].checked;
            toggleDisplay(elements[i]);
          }
        }
      }

      function filterFile(filter) {
        var elements = document.querySelectorAll(".fileEntry");

        for (var i = 0, len = elements.length; i < len; i++) {
          var visible = elements[i].querySelector("tr").querySelector("td").textContent.toLowerCase().includes(filter.toLowerCase());
          elements[i].classList.toggle("text-filtered", !visible);
        }
      }

      function filterText(text) {
        filter = text.toLowerCase();
        var elements = document.querySelectorAll(".issue");

        for (var i = 0, len = elements.length; i < len; i++) {
          var visible = false;
          var fields = elements[i].querySelectorAll("td");
          for (var n = 0, num = fields.length; n < num; n++) {
            if (fields[n].textContent.toLowerCase().includes(filter)) {
              visible = true;
              break;
            }
          }
          elements[i].classList.toggle("text-filtered", !visible);
        }

        updateFileRows();
      }

      function updateFileRows(element) {
        var elements = document.querySelectorAll(".fileEntry");

        for (var i = 0, len = elements.length; i < len; i++) {
          var visible = elements[i].querySelector(".issue:not(.id-filtered):not(.severity-filtered):not(.tool-filtered):not(.text-filtered)");
          elements[i].classList.toggle("file-filtered", !visible);
        }
      }

      window.addEventListener("load", initExpandables);
    </script>
  </head>
  <body>
    <div id="wrapper">
    <div id="header" class="header">
      <h1>Cppcheck report - %s%s</h1>
"""

HTML_HEAD_END = """
    </div>
"""

HTML_MENU = """
    <div id="menu">
     <p><a href="index.html">Defects:</a> %s</p>
"""

HTML_MENU_END = """
    </div>
    <div id="content">
"""

HTML_FOOTER = """
    </div>
    <div id="footer" class="footer">
      <p>
        Created by Cppcheck %s (<a href="https://cppcheck.sourceforge.io">Sourceforge</a>, <a href="irc://irc.freenode.net/cppcheck">IRC</a>)
      </p>
    </div>
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

def filter_button(enabled_filters, id, function):
    enabled = enabled_filters.get(id, False)
    return '\n        <label class="checkBtn%s"><input type="checkbox" onclick="%s(this)" id="%s"%s/>%s</label>'\
          % (' disabled' if not enabled else '', function, id, 'checked' if enabled else 'disabled', id)

def filter_bar(enabled):
    return ''.join([
     '      <div id="filters">\n'
    ,''.join([filter_button(enabled, severity, 'toggleSeverity') for severity in ['error', 'warning', 'portability', 'performance', 'style', 'information']])
    ,'\n        | '
    ,''.join([filter_button(enabled, tool, 'toggleTool') for tool in ['cppcheck', 'clang-tidy']])
    ,'\n        | '
    ,'\n        <label class="severityHeader">File: <input type="text" oninput="filterFile(this.value)"/></label>'
    ,'\n        <label class="severityHeader">Filter: <input type="text" oninput="filterText(this.value)"/></label>'
    ,'\n      </div>\n'])

def git_blame(errors, path, file, blame_options):
    last_line= errors[-1]['line']
    if last_line == 0:
        return {}

    first_line = next((error for error in errors if error['line'] > 0))['line']

    full_path = os.path.join(path, file)
    path, filename = os.path.split(full_path)

    if path:
        os.chdir(path)

    cmd_args = ['git', 'blame', '-L %d,%d' % (first_line, last_line)]
    if '-w' in blame_options:
        cmd_args.append('-w')
    if '-M' in blame_options:
        cmd_args.append('-M')
    cmd_args = cmd_args + ['--porcelain', '--incremental', '--', filename]

    try:
        result = subprocess.check_output(cmd_args)
        result = result.decode(locale.getpreferredencoding())
    except:
        return []
    finally:
        os.chdir(cwd)

    if result.startswith('fatal'):
        return []

    blame_data = []
    line_from = 0
    line_to = 0
    blame_info = dict()

    for line_str in result.splitlines():
        field, _, value = line_str.partition(' ')
        if len(field) == 40:
            line_nr, count = value.split(' ')[1:]
            line_from = int(line_nr)
            line_to = line_from + int(count)
        elif field.startswith('author'):
            blame_info[field] = value
            if field == 'author-time':
                author_date = date.fromtimestamp(int(blame_info['author-time']))
                blame_info['author-time'] = author_date.strftime("%d/%m/%Y")
        elif field == 'filename':
            blame_data.append([line_from, line_to, blame_info.copy()])

    return blame_data


def blame_lookup(blame_data, line):
    return next((data for start, end, data in blame_data if line >= start and line < end), {})


def tr_str(td_th, line, id, cwe, severity, message, author, author_mail, date, add_author, tr_class=None, htmlfile=None, message_class=None):
    ret = ''
    if htmlfile:
        ret += '<%s><a href="%s#line-%d">%d</a></%s>' % (td_th, htmlfile, line, line, td_th)
        for item in (id, cwe, severity):
            ret += '<%s>%s</%s>' % (td_th, item, td_th)
    else:
        for item in (line, id, cwe, severity):
            ret += '<%s>%s</%s>' % (td_th, item, td_th)
    if message_class:
        message_attribute = ' class="%s"' % message_class
    else:
        message_attribute = ''
    ret += '<%s%s>%s</%s>' % (td_th, message_attribute, html_escape(message), td_th)

    for field in  add_author:
        if field == 'name':
            ret += '<%s>%s</%s>' % (td_th, html_escape(author), td_th)
        elif field == 'email':
            ret += '<%s>%s</%s>' % (td_th, html_escape(author_mail), td_th)
        elif field == 'date':
            ret += '<%s>%s</%s>' % (td_th, date, td_th)

    if tr_class:
        tr_attributes = ' class="%s"' % tr_class
    else:
        tr_attributes = ''
    return '<tr%s>%s</tr>' % (tr_attributes, ret)


def to_css_selector(tag):
    # https://www.w3.org/TR/CSS2/syndata.html#characters
    # Note: for our usage, we don't consider escaped characters
    invalid_css_chars = re.compile(r"[^-_a-zA-Z0-9\u00A0-\uFFFF]")
    invalid_start = re.compile(r"^([0-9]|-[0-9]|--)")
    # Replace forbidden characters by an hyphen
    valid_chars = invalid_css_chars.sub("-", tag)
    # Check that the start of the tag doesn't break the rules
    start_invalid = invalid_start.match(valid_chars)
    if start_invalid:
        # otherwise, append a token to make it valid
        valid_chars_valid_start = "cpp" + valid_chars
    else:
        valid_chars_valid_start = valid_chars
    return valid_chars_valid_start

class AnnotateCodeFormatter(HtmlFormatter):
    errors = []

    def wrap(self, *args, **kwargs):
        line_no = 1
        for i, t in HtmlFormatter.wrap(self, *args, **kwargs):
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
    parser.add_option('--file', dest='file', action="append",
                      help='The cppcheck xml output file to read defects '
                           'from. You can combine results from several '
                           'xml reports i.e. "--file file1.xml --file file2.xml ..". '
                           'Default is reading from stdin.')
    parser.add_option('--report-dir', dest='report_dir',
                      help='The directory where the HTML report content is '
                           'written.')
    parser.add_option('--source-dir', dest='source_dir',
                      help='Base directory where source code files can be '
                           'found.')
    parser.add_option('--add-author-information', dest='add_author_information',
                      help='Blame information to include. '
                           'Adds specified author information. '
                           'Specify as comma-separated list of either "name", "email", "date" or "n","e","d". '
                           'Default: "n,e,d"')
    parser.add_option('--source-encoding', dest='source_encoding',
                      help='Encoding of source code.', default='utf-8')
    parser.add_option('--blame-options', dest='blame_options',
                      help='[-w, -M] blame options which you can use to get author and author mail  '
                           '-w --> not including white spaces and returns original author of the line  '
                           '-M --> not including moving of lines and returns original author of the line')

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
    cwd = os.getcwd()
    source_dir = os.getcwd()
    if options.source_dir:
        source_dir = options.source_dir

    add_author_information = []
    if options.add_author_information:
        fields = [x.strip() for x in options.add_author_information.split(',')]
        for x in fields:
            if x.lower() in ['n', 'name']:
                add_author_information.append('name')
            elif x.lower() in ['e', 'email']:
                add_author_information.append('email')
            elif x.lower() in ['d', 'date']:
                add_author_information.append('date')
            else:
                print('Unrecognized value "%s" for author information, using default (name, email, date)' % x)
                add_author_information = ['name', 'email', 'date']
                break

    blame_options = ''
    if options.blame_options:
        blame_options = options.blame_options
        add_author_information = add_author_information or ['name', 'email', 'date']

    # Parse the xml from all files defined in file argument
    # or from stdin. If no input is provided, stdin is used
    # Produce a simple list of errors.
    print('Parsing xml report.')
    try:
        contentHandler = CppCheckHandler()
        for fname in options.file or [sys.stdin]:
            xml_parse(fname, contentHandler)
    except (XmlParseException, ValueError) as msg:
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
        if filename not in files:
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
            if error['id'] == 'unmatchedSuppression':
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
                               ': ' + filename))
            output_file.write(HTML_HEAD_END)

            output_file.write(HTML_MENU % (filename.split('/')[-1]))
            for error in sorted(errors, key=lambda k: k['line']):
                output_file.write("<a href=\"%s#line-%d\"> %s %s</a>" % (data['htmlfile'], error['line'], error['id'], error['line']))
            output_file.write(HTML_MENU_END)

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
        filter_enabled = {}
        for filename, data in sorted(files.items()):
            for error in data['errors']:
                stats.append(error['id'])  # get the stats
                filter_enabled[error['severity']] = True
                filter_enabled['clang-tidy' if error['id'].startswith('clang-tidy-') else 'cppcheck'] = True
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

        stat_fmt = "\n            <tr><td><input type=\"checkbox\" class=\"idToggle\" onclick=\"toggleDisplay(this)\" id=\"{}\" name=\"{}\" checked></td><td>{}</td><td>{}</td></tr>"
        for occurrences in reversed(range(cnt_min, cnt_max + 1)):
            for _id in [k for k, v in sorted(counter.items()) if v == occurrences]:
                stat_html.append(stat_fmt.format(
                    to_css_selector(_id),
                    to_css_selector(_id),
                    dict(counter.most_common())[_id],
                    _id
                ))

        output_file.write(HTML_HEAD % (options.title, '', options.title, ''))
        output_file.write(filter_bar(filter_enabled))
        output_file.write(HTML_HEAD_END)
        output_file.write(HTML_MENU.replace('id="menu"', 'id="menu_index"', 1).replace("Defects:", "Defect summary", 1) % (''))
        output_file.write('\n       <label><input type="checkbox" class=\"idToggle\" onclick="toggleAll()" checked> Toggle all</label>')
        output_file.write('\n       <table>')
        output_file.write('\n           <tr><th>Show</th><th>#</th><th>Defect ID</th></tr>')
        output_file.write(''.join(stat_html))
        output_file.write('\n           <tr><td></td><td>' + str(stats_count) + '</td><td>total</td></tr>')
        output_file.write('\n       </table>')
        output_file.write('\n       <p><a href="stats.html">Statistics</a></p>')
        output_file.write(HTML_MENU_END.replace("content", "content_index", 1))

        output_file.write('\n       <table class=\"summaryTable\">')
        output_file.write(
            '\n       %s' %
            tr_str('th', 'Line', 'Id', 'CWE', 'Severity', 'Message', 'Author', 'Author mail', 'Date (DD/MM/YYYY)', add_author=add_author_information))

        for filename, data in sorted(files.items()):
            file_error = filename in decode_errors or filename.endswith('*')
            is_file = filename != '' and not file_error
            row_content = filename if file_error else "<a href=\"%s\">%s</a>" % (data['htmlfile'], filename)
            htmlfile = data.get('htmlfile') if is_file else None

            output_file.write("\n      <tbody class=\"fileEntry\">")
            output_file.write("\n       <tr><td colspan=\"5\">%s</td></tr>" % row_content)

            if filename in decode_errors:
                output_file.write("\n       <tr><td colspan=\"5\">Could not generated due to UnicodeDecodeError</td></tr>")

            sorted_errors = sorted(data['errors'], key=lambda k: k['line'])
            blame_data = git_blame(sorted_errors, source_dir, filename, blame_options) if add_author_information else []
            for error in sorted_errors:
                git_blame_dict = blame_lookup(blame_data, error['line'])
                message_class = None
                try:
                    if error['inconclusive'] == 'true':
                        message_class = 'inconclusive'
                        error['severity'] += ", inconcl."
                except KeyError:
                    pass

                try:
                    if error['cwe']:
                        cwe_url = "<a href=\"https://cwe.mitre.org/data/definitions/" + error['cwe'] + ".html\">" + error['cwe'] + "</a>"
                except KeyError:
                    cwe_url = ""

                if error['severity'] in ['error', 'warning']:
                    message_class = error['severity']

                line = error["line"] if is_file else ""

                output_file.write(
                    '\n         %s' %
                    tr_str('td', line, error["id"], cwe_url, error["severity"], error["msg"],
                           git_blame_dict.get('author', 'Unknown'), git_blame_dict.get('author-mail', '---'),
                           git_blame_dict.get('author-time', '---'),
                           tr_class=to_css_selector(error["id"]) + ' sev_' + error["severity"] + ' issue',
                           message_class=message_class,
                           add_author=add_author_information,
                           htmlfile=htmlfile))
        output_file.write("\n      </tbody>")
        output_file.write('\n       </table>')
        output_file.write(HTML_FOOTER % contentHandler.versionCppcheck)

    if decode_errors:
        sys.stderr.write("\nGenerating html failed for the following files: " + ' '.join(decode_errors))
        sys.stderr.write("\nConsider changing source-encoding (for example: \"htmlreport ... --source-encoding=\"iso8859-1\"\"\n")

    print('Creating style.css file')
    os.chdir(cwd)       # going back to the cwd to find style.css
    with io.open(os.path.join(options.report_dir, 'style.css'), 'w') as css_file:
        css_file.write(STYLE_FILE)

    print("Creating stats.html (statistics)\n")
    stats_countlist = {}

    for filename, data in sorted(files.items()):
        if filename == '':
            continue
        stats_tmplist = []
        for error in sorted(data['errors'], key=lambda k: k['line']):
            stats_tmplist.append(error['severity'])

        stats_countlist[filename] = dict(Counter(stats_tmplist))

    # get top ten for each severity
    SEVERITIES = "error", "warning", "portability", "performance", "style", "unusedFunction", "information", "missingInclude", "internal"

    with io.open(os.path.join(options.report_dir, 'stats.html'), 'w') as stats_file:

        stats_file.write(HTML_HEAD % (options.title, '', options.title, ': Statistics'))
        stats_file.write(HTML_HEAD_END)

        stats_file.write(HTML_MENU.replace('id="menu"', 'id="menu_index"', 1).replace("Defects:", "Back to summary", 1) % (''))
        stats_file.write(HTML_MENU_END.replace("content", "content_index", 1))


        for sev in SEVERITIES:
            _sum = 0
            stats_templist = {}

            # if the we have an style warning but we are checking for
            # portability, we have to skip it to prevent KeyError
            try:
                for filename in stats_countlist:
                    try:  # also bail out if we have a file with no sev-results
                        _sum += stats_countlist[filename][sev]
                        stats_templist[filename] = int(stats_countlist[filename][sev])  # file : amount,
                    except KeyError:
                        continue
                # don't print "0 style" etc, if no style warnings were found
                if _sum == 0:
                    continue
            except KeyError:
                continue
            stats_file.write("<p><span class=\"statHeader\">Top 10 files for " + sev + " severity, total findings: " + str(_sum) + "</span><br>\n")

            # sort, so that the file with the most severities per type is first
            stats_list_sorted = sorted(stats_templist.items(), key=operator.itemgetter(1, 0), reverse=True)
            it = 0
            LENGTH = 0

            for i in stats_list_sorted:  # printing loop
                # for aesthetics: if it's the first iteration of the loop, get
                # the max length of the number string
                if it == 0:
                    LENGTH = len(str(i[1]))  # <- length of longest number, now get the difference and try to  make other numbers align to it

                stats_file.write("&#160;" * 3 + str(i[1]) + "&#160;" * (1 + LENGTH - len(str(i[1]))) + "<a href=\"" + files[i[0]]['htmlfile'] + "\">  " + i[0] + "</a><br>\n")
                it += 1
                if it == 10:  # print only the top 10
                    break
            stats_file.write("</p>\n")

        stats_file.write(HTML_FOOTER % contentHandler.versionCppcheck)

    print("\nOpen '" + options.report_dir + "/index.html' to see the results.")
