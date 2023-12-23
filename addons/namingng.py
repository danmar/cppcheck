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
#    "RE_VARNAME": "[a-z]*[a-zA-Z0-9_]*\\Z",
#    "RE_PRIVATE_MEMBER_VARIABLE": null,
#    "RE_FUNCTIONNAME": "[a-z0-9A-Z]*\\Z",
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
    def __init__(self, file, linenr, string):
        self.file = file
        self.linenr = linenr
        self.str = string
        self.column = 0

def reportNamingError(location,message,errorId='namingConvention',severity='style',extra=''):
    cppcheckdata.reportError(location,severity,message,'namingng',errorId,extra)

def loadConfig(configfile):
    with open(configfile) as fh:
        data = json.load(fh)
    return data


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

def check_include_guard_name(conf_ig,cfg,directive):
    parts = directive.str.split()
    if len(parts) != 2:
        msg = 'syntax error'
        reportNamingError(directive,msg,'syntax')
        return None
    guard_name = parts[1]

    filename = directive.file
    if conf_ig.get('input','path') == 'basename':
        filename = os.path.basename(filename)
    use_case = conf_ig.get('case','upper')
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
    expect_guard_name = conf_ig.get('prefix','') + barename + conf_ig.get('suffix','')
    if expect_guard_name != guard_name:
        msg = 'include guard naming violation; %s != %s'%(guard_name,expect_guard_name)
        reportNamingError(directive,msg,'includeGuardName')

    return guard_name

def check_include_guard(conf_ig,cfg,unguarded_include_files):
    # Scan for '#ifndef FILE_H' as the first directive, in the first N lines.
    # Then test whether the next directive #defines the found name.
    # Various tests are done:
    # - check include guards for their naming and consistency
    # - test whether include guards are in place
    max_linenr = conf_ig.get('max_linenr', 5)

    def report(directive,msg,errorId):
        reportNamingError(directive,msg,errorId)

    def report_pending_ifndef(directive):
        report(directive,'include guard #ifndef is not followed by #define','includeGuardIncomplete')

    last_fn = None
    pending_ifndef = None
    phase = 0
    for directive in cfg.directives:
        if last_fn != directive.file:
            if pending_ifndef:
                report_pending_ifndef(pending_ifndef)
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
            if phase == 0 and conf_ig.get('required',1):
                report(directive,'include guard not found before line %d'%max_linenr,'includeGuardMissing')
                pass
            phase = -1
            continue

        if phase == 0:
            # looking for '#ifndef FILE_H'
            if not directive.str.startswith('#ifndef'):
                if conf_ig.get('required',1):
                    report(directive,'first preprocessor directive should be include guard #ifndef','includeGuardMissing')
                phase = -1
                continue
            guard_name = check_include_guard_name(conf_ig,cfg,directive)
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
                report(directive,'include guard does not guard; %s != %s'%(guard_name,parts[1]),'includeGuardAwayFromDuty',severity='warning')

            unguarded_include_files.remove(directive.file)

            phase = -1
    if pending_ifndef:
        report_pending_ifndef(pending_ifndef)

def process(dumpfiles, configfile, debugprint=False):
    conf = loadConfig(configfile)

    if "include_guard" in conf and conf["include_guard"]:
        global include_guard_header_re
        include_guard_header_re = conf["include_guard"].get('RE_HEADERFILE',"[^/].*\\.h\\Z")

    for afile in dumpfiles:
        if not afile[-5:] == '.dump':
            continue
        if not args.cli:
            print('Checking ' + afile + '...')
        data = cppcheckdata.CppcheckData(afile)

        # Check File naming
        if "RE_FILE" in conf and conf["RE_FILE"]:
            for source_file in data.files:
                basename = os.path.basename(source_file)
                good = False
                for exp in conf["RE_FILE"]:
                    good |= bool(re.match(exp, source_file))
                    good |= bool(re.match(exp, basename))
                if not good:
                    mockToken = DataStruct(source_file, 0, basename)
                    reportNamingError(mockToken, 'File name ' + basename + ' violates naming convention')

        # Check Namespace naming
        if "RE_NAMESPACE" in conf and conf["RE_NAMESPACE"]:
            for tk in data.rawTokens:
                if tk.str == 'namespace':
                    mockToken = DataStruct(tk.next.file, tk.next.linenr, tk.next.str)
                    msgType = 'Namespace'
                    for exp in conf["RE_NAMESPACE"]:
                        evalExpr(conf["RE_NAMESPACE"], exp, mockToken, msgType)

        unguarded_include_files = []
        if "include_guard" in conf and conf["include_guard"]:
            if conf["include_guard"].get('required',1):
                unguarded_include_files = [fn for fn in data.files if re.match(include_guard_header_re,fn)]

        for cfg in data.configurations:
            if not args.cli:
                print('Checking %s, config %s...' % (afile, cfg.name))
            if "RE_VARNAME" in conf and conf["RE_VARNAME"]:
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

                        if conf["skip_one_char_variables"] and len(var.nameToken.str) == 1:
                            continue
                        if varType in conf.get("var_prefixes",{}):
                            if not var.nameToken.str.startswith(conf["var_prefixes"][varType]):
                                reportNamingError(var.typeStartToken,
                                                    'Variable ' +
                                                    var.nameToken.str +
                                                    ' violates naming convention')

                        mockToken = DataStruct(var.typeStartToken.file, var.typeStartToken.linenr, var.nameToken.str)
                        msgType = 'Variable'
                        for exp in conf["RE_VARNAME"]:
                            evalExpr(conf["RE_VARNAME"], exp, mockToken, msgType)

            # Check Private Variable naming
            if "RE_PRIVATE_MEMBER_VARIABLE" in conf and conf["RE_PRIVATE_MEMBER_VARIABLE"]:
                # TODO: Not converted yet
                for var in cfg.variables:
                    if (var.access is None) or var.access != 'Private':
                        continue
                    mockToken = DataStruct(var.typeStartToken.file, var.typeStartToken.linenr, var.nameToken.str)
                    msgType = 'Private member variable'
                    for exp in conf["RE_PRIVATE_MEMBER_VARIABLE"]:
                        evalExpr(conf["RE_PRIVATE_MEMBER_VARIABLE"], exp, mockToken, msgType)

            # Check Public Member Variable naming
            if "RE_PUBLIC_MEMBER_VARIABLE" in conf and conf["RE_PUBLIC_MEMBER_VARIABLE"]:
                for var in cfg.variables:
                    if (var.access is None) or var.access != 'Public':
                        continue
                    mockToken = DataStruct(var.typeStartToken.file, var.typeStartToken.linenr, var.nameToken.str)
                    msgType = 'Public member variable'
                    for exp in conf["RE_PUBLIC_MEMBER_VARIABLE"]:
                        evalExpr(conf["RE_PUBLIC_MEMBER_VARIABLE"], exp, mockToken, msgType)

            # Check Global Variable naming
            if "RE_GLOBAL_VARNAME" in conf and conf["RE_GLOBAL_VARNAME"]:
                for var in cfg.variables:
                    if (var.access is None) or var.access != 'Global':
                        continue
                    mockToken = DataStruct(var.typeStartToken.file, var.typeStartToken.linenr, var.nameToken.str)
                    msgType = 'Public member variable'
                    for exp in conf["RE_GLOBAL_VARNAME"]:
                        evalExpr(conf["RE_GLOBAL_VARNAME"], exp, mockToken, msgType)

            # Check Functions naming
            if "RE_FUNCTIONNAME" in conf and conf["RE_FUNCTIONNAME"]:
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

                        if retval and retval in conf.get("function_prefixes",{}):
                            if not token.function.name.startswith(conf["function_prefixes"][retval]):
                                reportNamingError(token, 'Function ' + token.function.name + ' violates naming convention')
                        mockToken = DataStruct(token.file, token.linenr, token.function.name)
                        msgType = 'Function'
                        for exp in conf["RE_FUNCTIONNAME"]:
                            evalExpr(conf["RE_FUNCTIONNAME"], exp, mockToken, msgType)

            # Check Class naming
            if "RE_CLASS_NAME" in conf and conf["RE_CLASS_NAME"]:
                for fnc in cfg.functions:
                    # Check if it is Constructor/Destructor
                    if fnc.type == 'Constructor' or fnc.type == 'Destructor':
                        mockToken = DataStruct(fnc.tokenDef.file, fnc.tokenDef.linenr, fnc.name)
                        msgType = 'Class ' + fnc.type
                        for exp in conf["RE_CLASS_NAME"]:
                            evalExpr(conf["RE_CLASS_NAME"], exp, mockToken, msgType)

            # Check include guard naming
            if "include_guard" in conf and conf["include_guard"]:
                check_include_guard(conf["include_guard"],cfg,unguarded_include_files)

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
