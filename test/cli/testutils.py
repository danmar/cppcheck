import logging
import os
import subprocess

# Create Cppcheck project file
import sys


def create_gui_project_file(project_file, root_path=None, import_project=None, paths=None, exclude_paths=None, suppressions=None, addon=None):
    cppcheck_xml = ('<?xml version="1.0" encoding="UTF-8"?>\n'
                    '<project version="1">\n')
    if root_path:
        cppcheck_xml += '  <root name="' + root_path + '"/>\n'
    if import_project:
        cppcheck_xml += '  <importproject>' + import_project + '</importproject>\n'
    if paths:
        cppcheck_xml += '  <paths>\n'
        for path in paths:
            cppcheck_xml += '    <dir name="' + path + '"/>\n'
        cppcheck_xml += '  </paths>\n'
    if exclude_paths:
        cppcheck_xml += '  <exclude>\n'
        for path in exclude_paths:
            cppcheck_xml += '    <path name="' + path + '"/>\n'
        cppcheck_xml += '  </exclude>\n'
    if suppressions:
        cppcheck_xml += '  <suppressions>\n'
        for suppression in suppressions:
            cppcheck_xml += '    <suppression'
            if 'fileName' in suppression:
                cppcheck_xml += ' fileName="' + suppression['fileName'] + '"'
            cppcheck_xml += '>' + suppression['id'] + '</suppression>\n'
        cppcheck_xml += '  </suppressions>\n'
    if addon:
        cppcheck_xml += '  <addons>\n'
        cppcheck_xml += '    <addon>%s</addon>\n' % addon
        cppcheck_xml += '  </addons>\n'
    cppcheck_xml += '</project>\n'

    f = open(project_file, 'wt')
    f.write(cppcheck_xml)
    f.close()


def __lookup_cppcheck_exe():
    # path the script is located in
    script_path = os.path.dirname(os.path.realpath(__file__))

    exe_name = "cppcheck"

    if sys.platform == "win32":
        exe_name += ".exe"

    exe_path = None

    if 'TEST_CPPCHECK_EXE_LOOKUP_PATH' in os.environ:
        lookup_paths = [os.environ['TEST_CPPCHECK_EXE_LOOKUP_PATH']]
    else:
        lookup_paths = [os.path.join(script_path, '..', '..'), '.']

    for base in lookup_paths:
        for path in ('', 'bin', os.path.join('bin', 'debug')):
            tmp_exe_path = os.path.join(base, path, exe_name)
            if os.path.isfile(tmp_exe_path):
                exe_path = tmp_exe_path
                break

    if exe_path:
        print("using '{}'".format(exe_path))
    return exe_path


# Run Cppcheck with args
def cppcheck(args, env=None, remove_checkers_report=True):
    exe = __lookup_cppcheck_exe()
    assert exe is not None, 'no cppcheck binary found'

    logging.info(exe + ' ' + ' '.join(args))
    p = subprocess.Popen([exe] + args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env)
    comm = p.communicate()
    stdout = comm[0].decode(encoding='utf-8', errors='ignore').replace('\r\n', '\n')
    stderr = comm[1].decode(encoding='utf-8', errors='ignore').replace('\r\n', '\n')
    if remove_checkers_report:
        if stderr.find('[checkersReport]\n') > 0:
            start_id = stderr.find('[checkersReport]\n')
            start_line = stderr.rfind('\n', 0, start_id)
            if start_line <= 0:
                stderr = ''
            else:
                stderr = stderr[:start_line + 1]
        elif stderr.find(': (information) Active checkers: ') >= 0:
            pos = stderr.find(': (information) Active checkers: ')
            if pos == 0:
                stderr = ''
            elif stderr[pos - 1] == '\n':
                stderr = stderr[:pos]
    return p.returncode, stdout, stderr


def assert_cppcheck(args, ec_exp=None, out_exp=None, err_exp=None, env=None):
    exitcode, stdout, stderr = cppcheck(args, env)
    if ec_exp is not None:
        assert exitcode == ec_exp, stdout
    if out_exp is not None:
        out_lines = stdout.splitlines()
        assert out_lines == out_exp, stdout
    if err_exp is not None:
        err_lines = stderr.splitlines()
        assert err_lines == err_exp, stderr
