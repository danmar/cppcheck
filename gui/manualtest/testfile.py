#!/usr/bin/env python3
"""Test files for run.py: JSON (structured, with test matrices) or the older markdown.

JSON format:

  {"title": "Project file dialog",
   "tests": [
     {"name": "Relative paths",
      "ticket": "#14983",                       optional
      "enabled": true,                          optional, default true
      "steps": ["Open the project dialog", "Select platform {platform}"],
      "expected": "Relative paths are used in the XML",
      "todo": "",                               optional, "cannot be automated" etc
      "matrix": {"platform": ["native", "unix32"], "lang": ["C", "C++"]}   optional
      "editions": ["premium"]                   optional, default all editions
     }]}

"editions" are the versions of the application that the test is for, a subset of EDITIONS. A run
is for one edition (run.py --edition, with the binary of that edition), the tests that are not for
it are left out. The script is the same for all editions, {edition} in steps/expected/todo/script
is the name of the edition of the run.

"script" is what run.py executes, see SCRIPT_DOC below. It is written by gen.py (an AI explores
the GUI once) or by hand, and is then replayed without any AI.

"projects": ["name", ...] are test projects, directories with files and code in DIR/projects/NAME,
shared by all tests next to them. Before every run they are copied to an empty temporary directory,
so a test always starts with the same files: {project:NAME} in a script is the copy of NAME, and
{project} when the test has exactly one project.

A script is made for the steps, expected result, todo, matrix and test projects (the files in them) of a test. "script_for" is a hash
of those fields at the time, when they are changed afterwards the script is OUTDATED: it is kept
but not run until it is generated again, edited or approved. "id" identifies a test when it is
renamed.

A test with a matrix is run once for every combination of the axis values (the cartesian
product). {axis} in steps/expected/todo is replaced by the value; {text} that is not an axis
is left as it is.

  testfile.py convert IN.md OUT.json     import a markdown test file
"""

import hashlib
import itertools
import json
import os
import re
import shutil
import sys

AXIS_RE = re.compile(r'^\w+$')
PROJECT_RE = re.compile(r'^[\w-]+$')
PROJECTS = 'projects'     # test projects: one directory per project, next to the test files
EDITIONS = ('opensource', 'premium')

SCRIPT_DOC = """A script is a list of steps. Every step is an object with "do" (an action) or "expect" (a check),
and an optional "label" that says what the step is for (shown when it fails). Elements are found
with name (substring of the accessible name/text, case-insensitive), role, path, nth, exact, like
the guitest.py commands. {work} is a temporary directory for files, {home} the HOME of the
application, {project:NAME} the copy of the test project NAME ({project} when the test has just one),
use those for the files of a test instead of write steps. {axis} values of the test matrix and {edition}
(opensource or premium, the same script is used for both) are replaced before the run.

actions
  {"do": "click|rightclick|doubleclick|focus|hover", "name": "...", "role": "...", "mouse": true}
  {"do": "settext", "name": "...", "value": "..."}      set the text of an edit field
  {"do": "type", "value": "..."}                         keyboard input to the focused element
  {"do": "key", "keys": ["ctrl+o", "Return"]}            xdotool key names
  {"do": "wait", "name": "...", "gone": true, "timeout": 10}
  {"do": "restart"}                                      quit and start the application again
  {"do": "sleep", "seconds": 1}
  {"do": "write", "path": "{work}/a.c", "content": "..."}
  {"do": "copy", "from": "{project:p}/a_v2.c", "to": "{project:p}/a.c"}   change a file of the project
checks, the test fails when one is wrong
  {"expect": "visible", "name": "...", "role": "...", "timeout": 5}
  {"expect": "gone", "name": "..."}
  {"expect": "state", "name": "...", "state": "checked"}   or "not_state": "checked"; also
                                                          showing enabled focused selected expanded
  {"expect": "value", "name": "...", "equals": "1"}        value or text of a field, a spin box...
  {"expect": "exited"}                                   the application has ended by itself, status 0 (Quit)
  {"expect": "text", "contains": "..."}                  or "not_contains": anywhere on screen
  {"expect": "ocr", "contains": "Recheck"}               text on the screen read by OCR (tesseract), or "not_contains";
                                                          only for what "text" cannot see, e.g. a context menu that
                                                          is not in the accessibility tree. Case is ignored.
  {"expect": "file", "path": "{home}/x.xml", "contains": "...", "not_contains": "...", "regex": "..."}
  {"expect": "alive"}
  {"expect": "capture", "name": "...", "role": "frame", "regex": "'Cppcheck (\\d[^']*)'", "as": "version"}
                                                          the regex is searched in the element's info line
                                                          (role 'name' text='...' value=...), group 1 (or the
                                                          whole match) is stored and {version} in the later
                                                          steps is replaced by it; use it instead of hard coding
                                                          text that changes, e.g. the version
A check can have "observed_fail": true when the application was seen not to do what the check says
(a known failure, only for the check that failed). Text checks wait up to "timeout" seconds (default 5).
Actions that need an element wait up to "timeout" seconds (default 5) for it to appear."""

SELECTOR = {'name', 'role', 'path', 'nth', 'exact'}
# step name -> (required keys, allowed optional keys)
STEPS = {
    'do': {
        'click': ((), SELECTOR | {'mouse'}), 'rightclick': ((), SELECTOR), 'doubleclick': ((), SELECTOR),
        'focus': ((), SELECTOR), 'hover': ((), SELECTOR),
        'settext': (('value',), SELECTOR), 'type': (('value',), set()), 'key': (('keys',), set()),
        'wait': ((), SELECTOR | {'gone'}), 'restart': ((), set()),
        'sleep': (('seconds',), set()), 'write': (('path', 'content'), set()), 'copy': (('from', 'to'), set()),
    },
    'expect': {
        'visible': ((), SELECTOR), 'gone': ((), SELECTOR), 'value': (('equals',), SELECTOR), 'exited': ((), set()),
        'state': ((), SELECTOR | {'state', 'not_state'}),
        'text': ((), {'contains', 'not_contains'}), 'file': (('path',), {'contains', 'not_contains', 'regex'}),
        'alive': ((), set()), 'ocr': ((), {'contains', 'not_contains'}),
        'capture': (('regex', 'as'), SELECTOR),
    },
}
NEEDS_SELECTOR = {'click', 'rightclick', 'doubleclick', 'focus', 'hover', 'settext', 'wait', 'visible', 'gone', 'state', 'value',
                  'capture'}
STATES = {'showing', 'enabled', 'focused', 'selected', 'expanded', 'checked', 'editable', 'modal'}


def validate_script(script, where='script'):
    if not isinstance(script, list):
        return [where + ': must be a list of steps']
    errors = []
    for i, step in enumerate(script, 1):
        w = '%s step %d' % (where, i)
        if not isinstance(step, dict):
            errors.append(w + ': not an object')
            continue
        kinds = [k for k in ('do', 'expect') if k in step]
        if len(kinds) != 1:
            errors.append(w + ': needs exactly one of "do" or "expect"')
            continue
        kind, action = kinds[0], step[kinds[0]]
        if action not in STEPS[kind]:
            errors.append('%s: unknown %s "%s", use one of %s' % (w, kind, action, ', '.join(STEPS[kind])))
            continue
        required, optional = STEPS[kind][action]
        allowed = set(required) | optional | {kind, 'label', 'timeout'} | ({'observed_fail'} if kind == 'expect' else set())
        for k in step:
            if k not in allowed:
                errors.append('%s: "%s" is not used by %s %s' % (w, k, kind, action))
        for k in required:
            if k not in step:
                errors.append('%s: %s %s needs "%s"' % (w, kind, action, k))
        if action in NEEDS_SELECTOR and not (SELECTOR - {'nth', 'exact'}) & set(step):
            errors.append('%s: %s %s needs name, role or path' % (w, kind, action))
        if action in ('text', 'ocr') and not {'contains', 'not_contains'} & set(step):
            errors.append(w + ': expect %s needs contains or not_contains' % action)
        if action == 'state' and not {'state', 'not_state'} & set(step):
            errors.append(w + ': expect state needs state or not_state')
        for k in ('state', 'not_state'):
            if k in step and step[k] not in STATES:
                errors.append('%s: unknown state "%s", use one of %s' % (w, step[k], ', '.join(sorted(STATES))))
        if action == 'capture':
            try:
                re.compile(step['regex'])
            except (re.error, TypeError) as e:
                errors.append('%s: bad regex: %s' % (w, e))
            if not re.fullmatch(r'[\w-]+', str(step['as'])):
                errors.append(w + ': "as" must be a name of letters, digits, _ and -')
    if script and not any('expect' in s for s in script if isinstance(s, dict)):
        errors.append(where + ': needs at least one "expect" step, otherwise nothing is tested')
    return errors


def from_markdown(text):
    m = re.search(r'^#\s+(.+)$', text, re.M)
    doc = {'title': m.group(1).strip() if m else '', 'tests': []}
    for m in re.finditer(r'^## Test:\s*(.+?)\s*\n(.*?)(?=^## |\Z)', text, re.S | re.M):
        test = {'name': m.group(1), 'steps': [], 'expected': '', 'todo': ''}
        field = None
        for line in m.group(2).splitlines():
            s = line.strip()
            if s.startswith('Ticket:'):
                test['ticket'] = s[len('Ticket:'):].strip()
                field = None
            elif s.startswith('EXPECTED:'):
                test['expected'] = s[len('EXPECTED:'):].strip()
                field = 'expected'
            elif s.startswith('TODO:'):
                test['todo'] = s[len('TODO:'):].strip()
                field = 'todo'
            elif not s:
                field = None
            elif field:
                test[field] += '\n' + s
            else:
                test['steps'].append(re.sub(r'^\d+\.\s+', '', s))
        doc['tests'].append(test)
    return doc


def validate(doc, root=None):
    """Return a list of error strings, empty when the document is fine. With ROOT (the directory of
    the test file) the test projects must exist."""
    errors = []
    if not isinstance(doc, dict) or not isinstance(doc.get('tests'), list):
        return ['the document needs a "tests" list']
    names = set()
    for i, t in enumerate(doc['tests'], 1):
        where = 'test %d' % i
        if not isinstance(t, dict):
            errors.append(where + ': not an object')
            continue
        name = t.get('name')
        if not isinstance(name, str) or not name.strip():
            errors.append(where + ': name is missing')
        elif name in names:
            errors.append('%s: duplicate name "%s"' % (where, name))
        else:
            names.add(name)
        if not isinstance(t.get('steps', []), list) or not all(isinstance(s, str) for s in t.get('steps', [])):
            errors.append(where + ': steps must be a list of strings')
        for k in ('expected', 'todo', 'ticket'):
            if not isinstance(t.get(k, ''), str):
                errors.append('%s: %s must be a string' % (where, k))
        projects = t.get('projects', [])
        if not isinstance(projects, list) or not all(isinstance(p, str) and PROJECT_RE.match(p) for p in projects):
            errors.append(where + ': projects must be a list of project names (letters, digits, _ and -)')
        elif root:
            errors += ['%s: the test project "%s" does not exist' % (where, p) for p in projects
                       if not os.path.isdir(os.path.join(root, PROJECTS, p))]
        editions = t.get('editions', list(EDITIONS))
        if (not isinstance(editions, list) or not editions or len(set(editions)) != len(editions)
                or not all(e in EDITIONS for e in editions)):
            errors.append('%s: editions must be a list of one or more of %s' % (where, ', '.join(EDITIONS)))
        errors += validate_script(t.get('script', []), where + ' script')
        matrix = t.get('matrix', {})
        if not isinstance(matrix, dict):
            errors.append(where + ': matrix must be an object')
            continue
        for axis, values in matrix.items():
            if not AXIS_RE.match(axis):
                errors.append('%s: axis name "%s" must be letters, digits and _' % (where, axis))
            if not isinstance(values, list) or not values or not all(isinstance(v, str) and v for v in values):
                errors.append('%s: axis "%s" needs a list of non-empty values' % (where, axis))
    return errors


def load(path):
    with open(path, encoding='utf-8') as f:
        text = f.read()
    return json.loads(text) if path.endswith('.json') else from_markdown(text)


def combinations(test):
    """The matrix of a test as a list of {axis: value}; [{}] when there is no matrix."""
    matrix = test.get('matrix') or {}
    axes = list(matrix)
    return [dict(zip(axes, values)) for values in itertools.product(*(matrix[a] for a in axes))]


def editions(test):
    """The editions that a test is for, all of them when it does not say."""
    return test.get('editions') or list(EDITIONS)


def substitute(text, combo):
    return re.sub(r'\{([\w:-]+)\}', lambda m: combo.get(m.group(1), m.group(0)), text)


def body(test, combo, edition=None):
    parts = []
    if test.get('ticket'):
        parts.append('Ticket: ' + test['ticket'])
    if edition:
        parts.append('Edition of the application: ' + edition)
    if combo:
        parts.append('Matrix values for this run: ' + ', '.join('%s=%s' % kv for kv in combo.items()))
    combo = dict(combo, edition=edition) if edition else combo
    if test.get('steps'):
        lines, n = [], 0
        for s in test['steps']:
            s = substitute(s, combo)
            if s.startswith(('* ', '- ')):   # a sub item of the step above
                lines.append('   ' + s)
            else:
                n += 1
                lines.append('%d. %s' % (n, s))
        parts.append('Steps:\n' + '\n'.join(lines))
    parts.append('EXPECTED: ' + substitute(test.get('expected', ''), combo))
    if test.get('todo'):
        parts.append('TODO: ' + substitute(test['todo'], combo))
    return '\n\n'.join(parts)


def project_files(root, name):
    """{relative path: bytes} of a test project."""
    base = os.path.join(root, PROJECTS, name)
    out = {}
    for dirpath, _, files in os.walk(base):
        for f in files:
            p = os.path.join(dirpath, f)
            with open(p, 'rb') as fh:
                out[os.path.relpath(p, base)] = fh.read()
    return out


def project_hash(root, name):
    h = hashlib.sha1()
    for path, data in sorted(project_files(root, name).items()):
        h.update(path.encode() + b'\0' + data + b'\0')
    return h.hexdigest()[:12]


def materialize(root, names, work):
    """Copy the test projects to WORK/projects/NAME. Returns the variables for the script."""
    variables = {}
    for name in names:
        src = os.path.join(root, PROJECTS, name)
        if not os.path.isdir(src):
            raise FileNotFoundError('the test project "%s" does not exist' % name)
        dst = os.path.join(work, PROJECTS, name)
        shutil.copytree(src, dst)
        variables['project:' + name] = dst
    if len(names) == 1:
        variables['project'] = variables['project:' + names[0]]
    return variables


def definition_hash(test, root=None):
    """What a script depends on: changing any of these, or a file of a test project, makes the script outdated."""
    d = {k: test.get(k) or ([] if k == 'steps' else {} if k == 'matrix' else '')
         for k in ('steps', 'expected', 'todo', 'matrix')}
    if test.get('projects'):
        d['projects'] = {p: project_hash(root, p) if root and os.path.isdir(os.path.join(root, PROJECTS, p)) else 'missing'
                         for p in test['projects']}
    return hashlib.sha1(json.dumps(d, sort_keys=True, ensure_ascii=False).encode()).hexdigest()[:12]


def ensure_ids(doc):
    for t in doc.get('tests', []):
        if not t.get('id'):
            t['id'] = 't' + hashlib.sha1(str(t.get('name', '')).encode()).hexdigest()[:8]
    return doc


def is_stale(test, root=None):
    """A script that was made for another version of the test. No script_for: old file, trusted."""
    return bool(test.get('script') and test.get('script_for') and test['script_for'] != definition_hash(test, root))


def stamp_scripts(doc, old, root=None):
    """Before saving. A new, edited or approved ("_approve") script is current for the test as it is
    now. An untouched script keeps what it was made for, so editing the test makes it outdated."""
    ensure_ids(doc)
    prev = {t['id']: t for t in ensure_ids(old or {}).get('tests', [])}
    for t in doc['tests']:
        t.pop('_stale', None)
        approve = t.pop('_approve', False)
        if not t.get('script'):
            t.pop('script_for', None)
            continue
        p = prev.get(t['id'])
        if approve or p is None or p.get('script') != t['script']:
            t['script_for'] = definition_hash(t, root)
        else:
            t['script_for'] = p.get('script_for') or definition_hash(p, root)
    return doc


def subst_deep(obj, combo):
    if isinstance(obj, str):
        return substitute(obj, combo)
    if isinstance(obj, list):
        return [subst_deep(x, combo) for x in obj]
    if isinstance(obj, dict):
        return {k: subst_deep(v, combo) for k, v in obj.items()}
    return obj


def expand(path, edition=None):
    """One dict {name, id, body, script, stale, combo} per matrix combination of every enabled test.
    With EDITION only the tests for that edition, and {edition} is replaced."""
    doc = ensure_ids(load(path))
    root = os.path.dirname(os.path.abspath(path))
    errors = validate(doc, root)
    if errors:
        sys.exit('error: %s: %s' % (path, '; '.join(errors)))
    out = []
    for t in doc['tests']:
        if not t.get('enabled', True) or edition and edition not in editions(t):
            continue
        for combo in combinations(t):
            name = t['name'] + (' [%s]' % ', '.join('%s=%s' % kv for kv in combo.items()) if combo else '')
            values = dict(combo, edition=edition) if edition else combo
            out.append({'name': name, 'id': t['id'], 'body': body(t, combo, edition), 'combo': combo, 'root': root,
                        'projects': t.get('projects', []), 'stale': is_stale(t, root),
                        'script': subst_deep(t.get('script', []), values)})
    return out


def save(path, doc):
    tmp = path + '.tmp'
    with open(tmp, 'w', encoding='utf-8') as f:
        json.dump(doc, f, indent=2, ensure_ascii=False)
        f.write('\n')
    os.replace(tmp, path)


if __name__ == '__main__':
    if len(sys.argv) != 4 or sys.argv[1] != 'convert':
        sys.exit(__doc__)
    save(sys.argv[3], load(sys.argv[2]))
