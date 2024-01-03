#!/usr/bin/env python3
#
# cppcheck addon for naming conventions
# An enhanced version. Configuration is taken from a json file
# It supports to check for type-based prefixes in function or variable names.
# Aside from include guard naming, include guard presence can also be tested.
#
# Example usage (variable name must start with lowercase, function name must start with uppercase):
# $ cppcheck --dump path-to-src/
# $ python namingng.py test.c.dump
#
# JSON format:
#
# {
#    "RE_VARNAME": ["[a-z]*[a-zA-Z0-9_]*\\Z"],
#    "RE_PRIVATE_MEMBER_VARIABLE": null,
#    "RE_FUNCTIONNAME": ["[a-z0-9A-Z]*\\Z"],
#    "_comment": "comments can be added to the config with underscore-prefixed keys",
#    "include_guard": {
#        "input": "path",
#        "prefix": "GUARD_",
#        "case": "upper",
#        "max_linenr": 5,
#        "RE_HEADERFILE": "[^/].*\\.h\\Z",
#        "required": true
#    },
#    "var_prefixes": {"uint32_t": "ui32"},
#    "function_prefixes": {"uint16_t": "ui16",
#                          "uint32_t": "ui32"}
# }
#
# RE_VARNAME, RE_PRIVATE_MEMBER_VARIABLE and RE_FUNCTIONNAME are regular expressions to cover the basic names
# In var_prefixes and function_prefixes there are the variable-type/prefix pairs

import cppcheckdata
import sys
import os
import re
import argparse
import json

# Auxiliary class
class DataStruct:
    def __init__(self, file, linenr, string, column=0):
        self.file = file
        self.linenr = linenr
        self.str = string
        self.column = column

def reportNamingError(location,message,errorId='namingConvention',severity='style',extra='',column=None):
    cppcheckdata.reportError(location,severity,message,'namingng',errorId,extra,columnOverride=column)

def configError(error,fatal=True):
    print('config error: %s'%error)
    if fatal:
        sys.exit(1)

def loadConfig(configfile):
    if not os.path.exists(configfile):
        configError("cannot find config file '%s'"%configfile)

    try:
        with open(configfile) as fh:
            try:
                data = json.load(fh)
            except json.JSONDecodeError as e:
                configError("error parsing config file as JSON at line %d: %s"%(e.lineno,e.msg))
    except Exception as e:
        configError("error opening config file '%s': %s"%(configfile,e))

    if not isinstance(data, dict):
        configError('config file must contain a JSON object at the top level')

    # All errors are emitted before bailing out, to make the unit test more
    # effective.
    have_error = False

    # Put config items in a class, so that settings can be accessed using
    # config.feature
    class Config:
        pass
    config = Config()

    mapping = {
        'file':                     ('RE_FILE', list),
        'namespace':                ('RE_NAMESPACE', list),
        'include_guard':            ('include_guard', dict),
        'variable':                 ('RE_VARNAME', list),
        'variable_prefixes':        ('var_prefixes', dict, {}),
        'private_member':           ('RE_PRIVATE_MEMBER_VARIABLE', list),
        'public_member':            ('RE_PUBLIC_MEMBER_VARIABLE', list),
        'global_variable':          ('RE_GLOBAL_VARNAME', list),
        'function_name':            ('RE_FUNCTIONNAME', list),
        'function_prefixes':        ('function_prefixes', dict, {}),
        'class_name':               ('RE_CLASS_NAME', list),
        'skip_one_char_variables':  ('skip_one_char_variables', bool),
    }

    # parse defined keys and store as members of config object
    for key,opts in mapping.items():
        json_key = opts[0]
        req_type = opts[1]
        default = None if len(opts)<3 else opts[2]

        value = data.pop(json_key,default)
        if value is not None and not isinstance(value, req_type):
            req_typename = req_type.__name__
            got_typename = type(value).__name__
            configError('%s must be %s (not %s), or not set'%(json_key,req_typename,got_typename),fatal=False)
            have_error = True
            continue

        if req_type == list and value is not None:
            # type 'list' implies a list of regular expressions
            for item in value:
                try:
                    re.compile(item)
                except re.error as err:
                    configError("item '%s' of '%s' is not a valid regular expression: %s"%(item,json_key,err),fatal=False)
                    have_error = True
            if have_error:
                continue

        setattr(config,key,value)

    # check remaining keys, only accept underscore-prefixed comments
    for key,value in data.items():
        if key == '' or key[0] != '_':
            configError("unknown config key '%s'"%key,fatal=False)
            have_error = True

    if have_error:
        sys.exit(1)

    return config


def checkTrueRegex(data, expr, msg):
    res = re.match(expr, data.str)
    if res:
        reportNamingError(data,msg)


def checkFalseRegex(data, expr, msg):
    res = re.match(expr, data.str)
    if not res:
        reportNamingError(data,msg)


def evalExpr(conf, exp, mockToken, msgType):
    if isinstance(conf, dict):
        if conf[exp][0]:
            msg = msgType + ' ' + mockToken.str + ' violates naming convention : ' + conf[exp][1]
            checkTrueRegex(mockToken, exp, msg)
        elif ~conf[exp][0]:
            msg = msgType + ' ' + mockToken.str + ' violates naming convention : ' + conf[exp][1]
            checkFalseRegex(mockToken, exp, msg)
        else:
            msg = msgType + ' ' + mockToken.str + ' violates naming convention : ' + conf[exp][0]
            checkFalseRegex(mockToken, exp, msg)
    else:
        msg = msgType + ' ' + mockToken.str + ' violates naming convention'
        checkFalseRegex(mockToken, exp, msg)

def check_include_guard_name(conf_include_guard,cfg,directive):
    parts = directive.str.split()
    if len(parts) != 2:
        msg = 'syntax error'
        reportNamingError(directive,msg,'syntax')
        return None,None
    guard_name = parts[1]
    guard_column = 1+directive.str.find(guard_name)

    filename = directive.file
    if conf_include_guard.get('input','path') == 'basename':
        filename = os.path.basename(filename)
    use_case = conf_include_guard.get('case','upper')
    if use_case == 'upper':
        filename = filename.upper()
    elif use_case == 'lower':
        filename = filename.lower()
    elif use_case == 'keep':
        pass # keep filename case as-is
    else:
        print("invalid config value for 'case': '%s'"%use_case,file=sys.stderr)
        sys.exit(1)

    barename = re.sub('[^A-Za-z0-9]','_',filename).strip('_')
    expect_guard_name = conf_include_guard.get('prefix','') + barename + conf_include_guard.get('suffix','')
    if expect_guard_name != guard_name:
        msg = 'include guard naming violation; %s != %s'%(guard_name,expect_guard_name)
        reportNamingError(directive,msg,'includeGuardName',column=guard_column)

    return guard_name,guard_column

def check_include_guard(conf_include_guard,cfg,unguarded_include_files):
    # Scan for '#ifndef FILE_H' as the first directive, in the first N lines.
    # Then test whether the next directive #defines the found name.
    # Various tests are done:
    # - check include guards for their naming and consistency
    # - test whether include guards are in place
    max_linenr = conf_include_guard.get('max_linenr', 5)

    def report(directive,msg,errorId,column=0):
        reportNamingError(directive,msg,errorId,column=column)

    def report_pending_ifndef(directive,column):
        report(directive,'include guard #ifndef is not followed by #define','includeGuardIncomplete',column=column)

    last_fn = None
    pending_ifndef = None
    phase = 0
    for directive in cfg.directives:
        if last_fn != directive.file:
            if pending_ifndef:
                report_pending_ifndef(pending_ifndef,guard_column)
                pending_ifndef = None
            last_fn = directive.file
            phase = 0
        if phase == -1:
            # ignore (the remainder of) this file
            continue
        if not re.match(include_guard_header_re,directive.file):
            phase = -1
            continue

        if directive.linenr > max_linenr:
            if phase == 0 and conf_include_guard.get('required',1):
                report(directive,'include guard not found before line %d'%max_linenr,'includeGuardMissing')
                pass
            phase = -1
            continue

        if phase == 0:
            # looking for '#ifndef FILE_H'
            if not directive.str.startswith('#ifndef'):
                if conf_include_guard.get('required',1):
                    report(directive,'first preprocessor directive should be include guard #ifndef','includeGuardMissing')
                phase = -1
                continue
            guard_name,guard_column = check_include_guard_name(conf_include_guard,cfg,directive)
            if guard_name == None:
                phase = -1
                continue
            pending_ifndef = directive
            phase = 1
        elif phase == 1:
            pending_ifndef = None
            # looking for '#define FILE_H'
            if not directive.str.startswith('#define'):
                report(directive,'second preprocessor directive should be include guard #define','includeGuardIncomplete')
                phase = -1
                continue
            parts = directive.str.split()
            if len(parts) == 1:
                report(directive,'syntax error','syntax')
                phase = -1
                continue
            if guard_name != parts[1]:
                report(directive,'include guard does not guard; %s != %s'%(guard_name,parts[1]),'includeGuardAwayFromDuty',severity='warning',column=guard_column)

            unguarded_include_files.remove(directive.file)

            phase = -1
    if pending_ifndef:
        report_pending_ifndef(pending_ifndef,guard_column)

def process(dumpfiles, configfile, debugprint=False):
    conf = loadConfig(configfile)

    if conf.include_guard:
        global include_guard_header_re
        include_guard_header_re = conf.include_guard.get('RE_HEADERFILE',"[^/].*\\.h\\Z")

    for afile in dumpfiles:
        if not afile[-5:] == '.dump':
            continue
        if not args.cli:
            print('Checking ' + afile + '...')
        data = cppcheckdata.CppcheckData(afile)

        # Check File naming
        if conf.file:
            for source_file in data.files:
                basename = os.path.basename(source_file)
                good = False
                for exp in conf.file:
                    good |= bool(re.match(exp, source_file))
                    good |= bool(re.match(exp, basename))
                if not good:
                    mockToken = DataStruct(source_file, 0, basename)
                    reportNamingError(mockToken, 'File name ' + basename + ' violates naming convention')

        # Check Namespace naming
        if conf.namespace:
            for tk in data.rawTokens:
                if tk.str == 'namespace':
                    mockToken = DataStruct(tk.next.file, tk.next.linenr, tk.next.str, tk.next.column)
                    msgType = 'Namespace'
                    for exp in conf.namespace:
                        evalExpr(conf.namespace, exp, mockToken, msgType)

        unguarded_include_files = []
        if conf.include_guard:
            if conf.include_guard.get('required',1):
                unguarded_include_files = [fn for fn in data.files if re.match(include_guard_header_re,fn)]

        for cfg in data.configurations:
            if not args.cli:
                print('Checking %s, config %s...' % (afile, cfg.name))
            if conf.variable:
                for var in cfg.variables:
                    if var.nameToken and var.access != 'Global' and var.access != 'Public' and var.access != 'Private':
                        prev = var.nameToken.previous
                        varType = prev.str
                        while "*" in varType and len(varType.replace("*", "")) == 0:
                            prev = prev.previous
                            varType = prev.str + varType

                        if debugprint:
                            print("Variable Name: " + str(var.nameToken.str))
                            print("original Type Name: " + str(var.nameToken.valueType.originalTypeName))
                            print("Type Name: " + var.nameToken.valueType.type)
                            print("Sign: " + str(var.nameToken.valueType.sign))
                            print("variable type: " + varType)
                            print("\n")
                            print("\t-- {} {}".format(varType, str(var.nameToken.str)))

                        if conf.skip_one_char_variables and len(var.nameToken.str) == 1:
                            continue
                        if varType in conf.variable_prefixes:
                            prefix = conf.variable_prefixes[varType]
                            if not var.nameToken.str.startswith(prefix):
                                reportNamingError(var.typeStartToken,
                                                    'Variable ' +
                                                    var.nameToken.str +
                                                    ' violates naming convention',
                                                    column=var.nameToken.column)

                        mockToken = DataStruct(var.typeStartToken.file, var.typeStartToken.linenr, var.nameToken.str, var.nameToken.column)
                        msgType = 'Variable'
                        for exp in conf.variable:
                            evalExpr(conf.variable, exp, mockToken, msgType)

            # Check Private Variable naming
            if conf.private_member:
                # TODO: Not converted yet
                for var in cfg.variables:
                    if (var.access is None) or var.access != 'Private':
                        continue
                    mockToken = DataStruct(var.typeStartToken.file, var.typeStartToken.linenr, var.nameToken.str, var.nameToken.column)
                    msgType = 'Private member variable'
                    for exp in conf.private_member:
                        evalExpr(conf.private_member, exp, mockToken, msgType)

            # Check Public Member Variable naming
            if conf.public_member:
                for var in cfg.variables:
                    if (var.access is None) or var.access != 'Public':
                        continue
                    mockToken = DataStruct(var.typeStartToken.file, var.typeStartToken.linenr, var.nameToken.str, var.nameToken.column)
                    msgType = 'Public member variable'
                    for exp in conf.public_member:
                        evalExpr(conf.public_member, exp, mockToken, msgType)

            # Check Global Variable naming
            if conf.global_variable:
                for var in cfg.variables:
                    if (var.access is None) or var.access != 'Global':
                        continue
                    mockToken = DataStruct(var.typeStartToken.file, var.typeStartToken.linenr, var.nameToken.str, var.nameToken.column)
                    msgType = 'Global variable'
                    for exp in conf.global_variable:
                        evalExpr(conf.global_variable, exp, mockToken, msgType)

            # Check Functions naming
            if conf.function_name:
                for token in cfg.tokenlist:
                    if token.function:
                        if token.function.type in ('Constructor', 'Destructor', 'CopyConstructor', 'MoveConstructor'):
                            continue
                        retval = token.previous.str
                        prev = token.previous
                        while "*" in retval and len(retval.replace("*", "")) == 0:
                            prev = prev.previous
                            retval = prev.str + retval
                        if debugprint:
                            print("\t:: {} {}".format(retval, token.function.name))

                        if retval and retval in conf.function_prefixes:
                            if not token.function.name.startswith(conf.function_prefixes[retval]):
                                reportNamingError(token, 'Function ' + token.function.name + ' violates naming convention', column=token.column)
                        mockToken = DataStruct(token.file, token.linenr, token.function.name, token.column)
                        msgType = 'Function'
                        for exp in conf.function_name:
                            evalExpr(conf.function_name, exp, mockToken, msgType)

            # Check Class naming
            if conf.class_name:
                for fnc in cfg.functions:
                    # Check if it is Constructor/Destructor
                    if fnc.type == 'Constructor' or fnc.type == 'Destructor':
                        mockToken = DataStruct(fnc.tokenDef.file, fnc.tokenDef.linenr, fnc.name, fnc.tokenDef.column)
                        msgType = 'Class ' + fnc.type
                        for exp in conf.class_name:
                            evalExpr(conf.class_name, exp, mockToken, msgType)

            # Check include guard naming
            if conf.include_guard:
                check_include_guard(conf.include_guard,cfg,unguarded_include_files)

        for fn in unguarded_include_files:
            mockToken = DataStruct(fn,0,os.path.basename(fn))
            reportNamingError(mockToken,'Missing include guard','includeGuardMissing')

if __name__ == "__main__":
    parser = cppcheckdata.ArgumentParser()
    parser.add_argument("--debugprint", action="store_true", default=False,
                        help="Add debug prints")
    parser.add_argument("--configfile", type=str, default="namingng.config.json",
                        help="Naming check config file")

    args = parser.parse_args()
    process(args.dumpfile, args.configfile, args.debugprint)

    sys.exit(0)
