
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


def lookup_cppcheck_exe():
    # path the script is located in
    script_path = os.path.dirname(os.path.realpath(__file__))

    exe_name = "cppcheck"

    if sys.platform == "win32":
        exe_name += ".exe"

    for base in (script_path + '/../../', './'):
        for path in ('', 'bin/', 'bin/debug/'):
            exe_path = base + path + exe_name
            if os.path.isfile(exe_path):
                print("using '{}'".format(exe_path))
                return exe_path

    return None


# Run Cppcheck with args
def cppcheck(args, env=None):
    exe = lookup_cppcheck_exe()
    assert exe is not None, 'no cppcheck binary found'

    logging.info(exe + ' ' + ' '.join(args))
    p = subprocess.Popen([exe] + args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env)
    comm = p.communicate()
    stdout = comm[0].decode(encoding='utf-8', errors='ignore').replace('\r\n', '\n')
    stderr = comm[1].decode(encoding='utf-8', errors='ignore').replace('\r\n', '\n')
    if stdout.find('\nActive checkers:') > 0:
        stdout = stdout[:1 + stdout.find('\nActive checkers:')]
    return p.returncode, stdout, stderr
