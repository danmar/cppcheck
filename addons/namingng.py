#!/usr/bin/env python3
#
# cppcheck addon for naming conventions
# An enhanced version. Configuration is taken from a json file
# It supports to check for type-based prefixes in function or variable names.
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


def process(dumpfiles, configfile, debugprint=False):
    conf = loadConfig(configfile)

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

if __name__ == "__main__":
    parser = cppcheckdata.ArgumentParser()
    parser.add_argument("--debugprint", action="store_true", default=False,
                        help="Add debug prints")
    parser.add_argument("--configfile", type=str, default="namingng.config.json",
                        help="Naming check config file")

    args = parser.parse_args()
    process(args.dumpfile, args.configfile, args.debugprint)

    sys.exit(0)
