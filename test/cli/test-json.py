
# python -m pytest test-json.py

import os
import sys
import pytest
import re
import json
import jsonschema

from pathlib import Path

def schema_mapping():
    def p(*args):
        return os.path.join(*args)
    return {
        p('addons','addon.schema.json'): [
            p('addons','namingng.json'),
            'naming.json'
        ],
        p('addons','addon-namingng-config.schema.json'): [
            p('addons','ROS_naming.json'),
            p('addons','namingng.config.json')
        ],
    }

# files that need no validation
def schema_whitelist():
    return [
        'compile_commands.json', # generated during build
    ]

def __load_json(js,label='JSON'):
    try:
        with open(js,'r') as f:
            return json.load(f)
    except json.JSONDecodeError as e:
        assert False,'Failed to parse %s %s at line %d: %s'%(label,js,e.lineno,e.msg)
    except Exception as e:
        assert False,'Failed to load %s %s: %s'%(label,js,e)

def validate_json(jsonfile,validator,schema_json_fn,json_fn,verbose=False):
    test_json = __load_json(jsonfile)
    issue = None
    try:
        validator.validate(instance=test_json)
    except jsonschema.exceptions.SchemaError as e:
        issue = 'JSON schema %s did not validate against metaschema'%(schema_json_fn)
        if verbose:
            # this will also emit the SchemaError
            assert False, issue
    except jsonschema.exceptions.ValidationError as e:
        issue = 'JSON %s did not validate against schema %s'%(json_fn,schema_json_fn)
        if verbose:
            # this will also emit the ValidationError
            assert False, issue
    except Exception as e:
        # In some cases a different exception is raised from deep within jsonschema
        issue = 'An issue occurred validating %s against %s'%(json_fn,schema_json_fn)
        if verbose:
            # this will also emit the unhandled error
            assert False, issue

    assert issue == None

# we could use @pytest.mark.parametrize('schema', schema_mapping()), to make it
# one test per schema, BUT that would lead to a lot of double code as we also
# test whether all JSON files are covered.
def test_schemas(request):
    verbose = request.config.option.verbose
    script_path = Path(os.path.realpath(__file__))
    assert str(script_path.parents[0].name) == 'cli'
    assert str(script_path.parents[1].name) == 'test'
    cppcheck_dir = script_path.parents[2]

    def include(p):
        # This is a bit clunky but there is no clear separation between
        # codebase and build/install directories on all CI instances.
        relpath = p.relative_to(cppcheck_dir)
        parents = list(relpath.parents)
        if len(parents)>1 and parents[-2].name in ('bin','build','cmake.output'):
            return False
        return True

    json_files_all = set(cppcheck_dir.rglob('*.json'))
    json_files = [p.resolve() for p in cppcheck_dir.rglob('*.json') if include(p)]

    format_checker = jsonschema.FormatChecker()
    @format_checker.checks('python_re', AssertionError)
    def is_python_re(value):
        if not isinstance(value,str):
            issue = 'Expect string containing regular expression, got "%s"'%(str(value))
            if verbose:
                raise Exception(issue)
            else:
                assert False, issue
        issue = None
        try:
            re.compile(value)
        except re.error as err:
            issue = 'Error compiling Python regular expression "%s": %s'%(value,str(err))
        if issue and verbose:
            # raise an exception describing the issue, so that it is visible if testing --verbose
            raise Exception(issue)
        assert issue == None
        return True

    recent_validator = jsonschema.validators.validator_for(True)
    validator_factory = jsonschema.validators.create(
        meta_schema=recent_validator.META_SCHEMA,
        validators=recent_validator.VALIDATORS
    )

    for schema_json_fn,jsons in schema_mapping().items():
        schemafile = cppcheck_dir.joinpath(schema_json_fn)
        assert schemafile.exists(), 'schema file %s not found'%schema_json_fn
        json_files.remove(schemafile)
        schema_json = __load_json(schemafile,'JSON schema')
        # Note that there is no need to validate the schema against
        # the metaschema here; this is already done by jsonschema
        # as a first step in jsonschema.validate().

        validator = validator_factory(schema_json, format_checker=format_checker)

        for json_fn in jsons:
            jsonfile = cppcheck_dir.joinpath(json_fn)
            assert jsonfile.exists(), 'JSON file %s not found'%json_fn
            json_files.remove(jsonfile)
            validate_json(jsonfile,validator,schema_json_fn,json_fn,verbose=request.config.option.verbose)

    whitelist = schema_whitelist()
    for f in list(json_files):
        if f.name in whitelist:
            json_files.remove(f)

    assert len(json_files)==0, 'JSON files without schema found: %s'%(', '.join([str(p) for p in json_files]))