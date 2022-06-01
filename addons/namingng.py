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
import re
import argparse
import json


# Auxiliary class
class DataStruct:
    def __init__(self, file, linenr, string):
        self.file = file
        self.linenr = linenr
        self.str = string


def reportError(filename, linenr, severity, msg):
    message = "[{filename}:{linenr}] ( {severity} ) naming.py: {msg}\n".format(
        filename=filename,
        linenr=linenr,
        severity=severity,
        msg=msg
    )
    sys.stderr.write(message)
    return message


def loadConfig(configfile):
    with open(configfile) as fh:
        data = json.load(fh)
    return data


def checkTrueRegex(data, expr, msg, errors):
    res = re.match(expr, data.str)
    if res:
        errors.append(reportError(data.file, data.linenr, 'style', msg))


def checkFalseRegex(data, expr, msg, errors):
    res = re.match(expr, data.str)
    if not res:
        errors.append(reportError(data.file, data.linenr, 'style', msg))


def evalExpr(conf, exp, mockToken, msgType, errors):
    if isinstance(conf, dict):
        if conf[exp][0]:
            msg = msgType + ' ' + mockToken.str + ' violates naming convention : ' + conf[exp][1]
            checkTrueRegex(mockToken, exp, msg, errors)
        elif ~conf[exp][0]:
            msg = msgType + ' ' + mockToken.str + ' violates naming convention : ' + conf[exp][1]
            checkFalseRegex(mockToken, exp, msg, errors)
        else:
            msg = msgType + ' ' + mockToken.str + ' violates naming convention : ' + conf[exp][0]
            checkFalseRegex(mockToken, exp, msg, errors)
    else:
        msg = msgType + ' ' + mockToken.str + ' violates naming convention'
        checkFalseRegex(mockToken, exp, msg, errors)


def process(dumpfiles, configfile, debugprint=False):

    errors = []

    conf = loadConfig(configfile)

    for afile in dumpfiles:
        if not afile[-5:] == '.dump':
            continue
        print('Checking ' + afile + '...')
        data = cppcheckdata.CppcheckData(afile)

        # Check File naming
        if "RE_FILE" in conf and conf["RE_FILE"]:
            mockToken = DataStruct(afile[:-5], "0", afile[afile.rfind('/') + 1:-5])
            msgType = 'File name'
            for exp in conf["RE_FILE"]:
                evalExpr(conf["RE_FILE"], exp, mockToken, msgType, errors)

        # Check Namespace naming
        if "RE_NAMESPACE" in conf and conf["RE_NAMESPACE"]:
            for tk in data.rawTokens:
                if tk.str == 'namespace':
                    mockToken = DataStruct(tk.next.file, tk.next.linenr, tk.next.str)
                    msgType = 'Namespace'
                    for exp in conf["RE_NAMESPACE"]:
                        evalExpr(conf["RE_NAMESPACE"], exp, mockToken, msgType, errors)

        for cfg in data.configurations:
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
                        if varType in conf["var_prefixes"]:
                            if not var.nameToken.str.startswith(conf["var_prefixes"][varType]):
                                errors.append(reportError(
                                                    var.typeStartToken.file,
                                                    var.typeStartToken.linenr,
                                                    'style',
                                                    'Variable ' +
                                                    var.nameToken.str +
                                                    ' violates naming convention'))

                        mockToken = DataStruct(var.typeStartToken.file, var.typeStartToken.linenr, var.nameToken.str)
                        msgType = 'Variable'
                        for exp in conf["RE_VARNAME"]:
                            evalExpr(conf["RE_VARNAME"], exp, mockToken, msgType, errors)

            # Check Private Variable naming
            if "RE_PRIVATE_MEMBER_VARIABLE" in conf and conf["RE_PRIVATE_MEMBER_VARIABLE"]:
                # TODO: Not converted yet
                for var in cfg.variables:
                    if (var.access is None) or var.access != 'Private':
                        continue
                    mockToken = DataStruct(var.typeStartToken.file, var.typeStartToken.linenr, var.nameToken.str)
                    msgType = 'Private member variable'
                    for exp in conf["RE_PRIVATE_MEMBER_VARIABLE"]:
                        evalExpr(conf["RE_PRIVATE_MEMBER_VARIABLE"], exp, mockToken, msgType, errors)

            # Check Public Member Variable naming
            if "RE_PUBLIC_MEMBER_VARIABLE" in conf and conf["RE_PUBLIC_MEMBER_VARIABLE"]:
                for var in cfg.variables:
                    if (var.access is None) or var.access != 'Public':
                        continue
                    mockToken = DataStruct(var.typeStartToken.file, var.typeStartToken.linenr, var.nameToken.str)
                    msgType = 'Public member variable'
                    for exp in conf["RE_PUBLIC_MEMBER_VARIABLE"]:
                        evalExpr(conf["RE_PUBLIC_MEMBER_VARIABLE"], exp, mockToken, msgType, errors)

            # Check Global Variable naming
            if "RE_GLOBAL_VARNAME" in conf and conf["RE_GLOBAL_VARNAME"]:
                for var in cfg.variables:
                    if (var.access is None) or var.access != 'Global':
                        continue
                    mockToken = DataStruct(var.typeStartToken.file, var.typeStartToken.linenr, var.nameToken.str)
                    msgType = 'Public member variable'
                    for exp in conf["RE_GLOBAL_VARNAME"]:
                        evalExpr(conf["RE_GLOBAL_VARNAME"], exp, mockToken, msgType, errors)

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

                        if retval and retval in conf["function_prefixes"]:
                            if not token.function.name.startswith(conf["function_prefixes"][retval]):
                                errors.append(reportError(
                                    token.file, token.linenr, 'style', 'Function ' + token.function.name + ' violates naming convention'))
                        mockToken = DataStruct(token.file, token.linenr, token.function.name)
                        msgType = 'Function'
                        for exp in conf["RE_FUNCTIONNAME"]:
                            evalExpr(conf["RE_FUNCTIONNAME"], exp, mockToken, msgType, errors)

            # Check Class naming
            if "RE_CLASS_NAME" in conf and conf["RE_CLASS_NAME"]:
                for fnc in cfg.functions:
                    # Check if it is Constructor/Destructor
                    if fnc.type == 'Constructor' or fnc.type == 'Destructor':
                        mockToken = DataStruct(fnc.tokenDef.file, fnc.tokenDef.linenr, fnc.name)
                        msgType = 'Class ' + fnc.type
                        for exp in conf["RE_CLASS_NAME"]:
                            evalExpr(conf["RE_CLASS_NAME"], exp, mockToken, msgType, errors)
    return errors


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Naming verification')
    parser.add_argument('dumpfiles', type=str, nargs='+',
                        help='A set of dumpfiles to process')
    parser.add_argument("--debugprint", action="store_true", default=False,
                        help="Add debug prints")
    parser.add_argument("--configfile", type=str, default="naming.json",
                        help="Naming check config file")
    parser.add_argument("--verify", action="store_true", default=False,
                        help="verify this script. Must be executed in test folder !")

    args = parser.parse_args()
    errors = process(args.dumpfiles, args.configfile, args.debugprint)

    if args.verify:
        print(errors)
        if len(errors) < 6:
            print("Not enough errors found")
            sys.exit(1)
        target = [
         '[namingng_test.c:8] ( style ) naming.py: Variable badui32 violates naming convention\n',
         '[namingng_test.c:11] ( style ) naming.py: Variable a violates naming convention\n',
         '[namingng_test.c:29] ( style ) naming.py: Variable badui32 violates naming convention\n',
         '[namingng_test.c:20] ( style ) naming.py: Function ui16bad_underscore violates naming convention\n',
         '[namingng_test.c:25] ( style ) naming.py: Function u32Bad violates naming convention\n',
         '[namingng_test.c:37] ( style ) naming.py: Function Badui16 violates naming convention\n']
        diff = set(errors) - set(target)
        if len(diff):
            print("Not the right errors found {}".format(str(diff)))
            sys.exit(1)
        print("Verification done\n")
        sys.exit(0)

    if len(errors):
        print('Found errors: {}'.format(len(errors)))
        sys.exit(1)

    sys.exit(0)
