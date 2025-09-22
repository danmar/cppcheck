# Running the test with Python 3:
# Command in cppcheck directory:
# PYTHONPATH=./addons python3 -m pytest addons/test/unionzeroinit.py

import contextlib
import sys

from addons import cppcheckdata
from addons import unionzeroinit
from . import util


@contextlib.contextmanager
def run(path):
    sys.argv.append("--cli")
    util.dump_create(path)
    data = cppcheckdata.CppcheckData(f"{path}.dump")
    for cfg in data.iterconfigurations():
        yield cfg, data
    sys.argv.remove("--cli")
    util.dump_remove(path)


def test_basic(capsys):
    with run("./addons/test/unionzeroinit/basic.c") as (cfg, data):
        unionzeroinit.union_zero_init(cfg, data)
        captured = capsys.readouterr()
        captured = captured.out.splitlines()
        json_output = util.convert_json_output(captured)
        assert len(json_output["unionzeroinit"]) == 3
        assert json_output["unionzeroinit"][0]["linenr"] == 22
        assert json_output["unionzeroinit"][1]["linenr"] == 23
        assert json_output["unionzeroinit"][2]["linenr"] == 25


def test_array_member(capsys):
    with run("./addons/test/unionzeroinit/array.c") as (cfg, data):
        unionzeroinit.union_zero_init(cfg, data)
        captured = capsys.readouterr()
        captured = captured.out.splitlines()
        json_output = util.convert_json_output(captured)
        assert len(json_output) == 0


def test_struct_member(capsys):
    with run("./addons/test/unionzeroinit/struct.c") as (cfg, data):
        unionzeroinit.union_zero_init(cfg, data, True)
        captured = capsys.readouterr()
        captured = captured.out.splitlines()
        json_output = util.convert_json_output(captured)
        assert len(json_output) == 0


def test_struct_cyclic_member(capsys):
    with run("./addons/test/unionzeroinit/struct-cyclic.c") as (cfg, data):
        unionzeroinit.union_zero_init(cfg, data, True)
        captured = capsys.readouterr()
        captured = captured.out.splitlines()
        json_output = util.convert_json_output(captured)
        assert len(json_output) == 0


def test_unknown_type(capsys):
    with run("./addons/test/unionzeroinit/unknown-type.c") as (cfg, data):
        unionzeroinit.union_zero_init(cfg, data, True)
        captured = capsys.readouterr()
        captured = captured.out.splitlines()
        json_output = util.convert_json_output(captured)
        assert len(json_output) == 0


def test_long_long(capsys):
    with run("./addons/test/unionzeroinit/long-long.c") as (cfg, data):
        unionzeroinit.union_zero_init(cfg, data, True)
        captured = capsys.readouterr()
        captured = captured.out.splitlines()
        json_output = util.convert_json_output(captured)
        assert len(json_output) == 0


def test_bitfields(capsys):
    with run("./addons/test/unionzeroinit/bitfields.c") as (cfg, data):
        unionzeroinit.union_zero_init(cfg, data, True)
        captured = capsys.readouterr()
        captured = captured.out.splitlines()
        json_output = util.convert_json_output(captured)
        assert len(json_output) == 0
