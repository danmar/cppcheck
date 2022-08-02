#!/usr/bin/env python
import os.path
import subprocess
import sys
#import difflib
import argparse

from packaging.version import Version

parser = argparse.ArgumentParser()
parser.add_argument('dir', help='directory with versioned folders')
parser.add_argument('infile', help='the file to analyze')
parser.add_argument('repo', nargs='?', default=None, help='the git repository (for sorting commit hashes)')
parser.add_argument('--compare', action='store_true')
parser.add_argument('--verbose', action='store_true')
#parser.add_argument('--diff', action='store_true') # TODO
#parser.add_argument('--bisect', action='store_true') # TODO: invoke a bisect on the last difference
args = parser.parse_args()

def sort_commit_hashes(commits):
    git_cmd = 'git rev-list --abbrev-commit --topo-order --no-walk=sorted --reverse ' + ' '.join(commits)
    p = subprocess.Popen(git_cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=git_repo, universal_newlines=True)
    comm = p.communicate()
    return comm[0].splitlines()

verbose = args.verbose
do_compare = args.compare
#differ = difflib.Differ()

directory = args.dir
input_file = args.infile
git_repo = args.repo

use_hashes = None
versions = []

for filename in os.listdir(directory):
    f = os.path.join(directory, filename)
    if not os.path.isdir(f):
        continue
    versions.append(filename)

if len(versions):
    try:
        Version(versions[0])
        use_hashes = False
        versions.sort(key=Version)
    except:
        print("'{}' not a version - assuming commit hashes".format(versions[0]))
        if not git_repo:
            print('error: git repository argument required for commit hash sorting')
            sys.exit(1)
        use_hashes = True
        # if you use the folder from the bisect script that contains the repo as a folder - so remove it from the list
        if versions.count('cppcheck'):
            versions.remove('cppcheck')
        versions = sort_commit_hashes(versions)

last_ec = None
last_out = None

for version in versions:
    exe_path = os.path.join(directory, version)
    exe = os.path.join(exe_path, 'cppcheck')
    cmd = exe
    cmd += ' '
    if do_compare:
        cmd += ' -q '
    #cmd += ' --language=c '
    # TODO: get version for each commit
    if not use_hashes:
        #if Version(version) >= Version('1.45'):
        #    cmd += '--debug-warnings --debug '
        #if Version(version) >= Version('1.61'):
        #    cmd += '--check-library '
        if Version(version) >= Version('1.39'):
            cmd += '--enable=all '
        if Version(version) >= Version('1.40'):
            cmd += '--inline-suppr '
        if Version(version) >= Version('1.48'):
            cmd += '--suppress=missingInclude --suppress=missingIncludeSystem --suppress=unmatchedSuppression --suppress=unusedFunction '
        if Version(version) >= Version('1.49'):
            cmd += '--inconclusive '
    cmd += input_file
    p = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=exe_path, universal_newlines=True)
    try:
        comm = p.communicate(timeout=2)
    except subprocess.TimeoutExpired:
        print('timeout')
        p.kill()
        comm = p.communicate()

    ec = p.returncode
    out = comm[0] + '\n' + comm[1]

    if not do_compare:
        print(version)
        print(ec)
        print(out)
        continue

    # filter out some false positives
    # [*]: (information) Unmatched suppression: missingInclude
    # [*]: (information) Unmatched suppression: missingIncludeSystem
    # [*]: (information) Unmatched suppression: unmatchedSuppression
    # [*]: (information) Unmatched suppression: unusedFunction
    if not use_hashes and (Version(version) >= Version('1.48') or Version(version) <= Version('1.49')):
        lines = out.splitlines()
        out = ""
        for line in lines:
            if line.startswith('[*]: (information) Unmatched suppression:'):
                continue
            out += line + '\n'

    out = out.strip()

    if last_ec is None:
        # first run - do nothing
        last_ec = ec
        last_out = out
        continue

    do_print = False

    if last_ec != ec:
        if verbose:
            print("{}: exitcode changed".format(version))
        do_print = True

    if last_out != out:
        if verbose:
            print("{}: output changed".format(version))
        do_print = True
        #print('\n'.join(differ.compare([last_out], [out])))

    if do_print:
        print(last_ec)
        print(last_out)
    print(version)

    last_ec = ec
    last_out = out

print(last_ec)
print(last_out)