# Helpers for pytest tests
import subprocess
import json

def dump_create(fpath, *argv):
    cmd = ["./cppcheck", "--dump", "--quiet", fpath] + list(argv)
    p = subprocess.Popen(cmd)
    p.communicate()
    if p.returncode != 0:
        raise OSError("cppcheck returns error code: %d" % p.returncode)
    subprocess.Popen(["sync"])


def dump_remove(fpath):
    subprocess.Popen(["rm", "-f", fpath + ".dump"])


def convert_json_output(raw_json_strings):
    """Convert raw stdout/stderr cppcheck JSON output to python dict."""
    json_output = {}
    for line in raw_json_strings:
        try:
            json_line = json.loads(line)
            #  json_output[json_line['errorId']] = json_line
            json_output.setdefault(json_line['errorId'], []).append(json_line)
        except ValueError:
            pass
    return json_output
