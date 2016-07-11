This folder is for triage data of cppcheck results.

You can scan these projects with arbitrary cppcheck version and get triaged reports.

Usage:

1. Pick a project, for example linux-3.11.
2. Scan linux-3.11 on your computer with cppcheck (arbitrary version).
3. run the triage.py script:
    python triage.py linux-3.11 path-to-cppcheck-results.txt
4. A report.html is generated
