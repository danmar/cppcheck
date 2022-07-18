#!/usr/bin/env python
import os.path
import subprocess
import sys

from packaging.version import Version

# TODO: add a mode which actually diffs the output and only reports differences

def sort_commit_hashes(commits):
    git_cmd = 'git rev-list --abbrev-commit --topo-order --no-walk=sorted --reverse ' + ' '.join(commits)
    p = subprocess.Popen(git_cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=git_repo, universal_newlines=True)
    comm = p.communicate()
    return comm[0].splitlines()

argc = len(sys.argv)

if argc < 3 and argc > 4:
    print("Usage: diff.py <dir-to-versions> <input-file> [git-repo]")
    sys.exit(1)

directory = sys.argv[1]
input_file = sys.argv[2]
git_repo = None
if argc == 4:
    git_repo = sys.argv[3]

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

for version in versions:
    exe_path = os.path.join(directory, version)
    exe = os.path.join(exe_path, 'cppcheck')
    cmd = exe
    cmd += ' '
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
    print(version)
    print(p.returncode)
    print(comm[0] + '\n' + comm[1])