## @mainpage cppcheckdata
#
# @brief This is a Python module that helps you access Cppcheck dump data.<br><br>
#
# License: No restrictions, use this as you need.<br><br>
#

import xml.etree.ElementTree as ET
import argparse

## Directive class. Contains information about each preprocessor directive in the source code.
#
# file and linenr denote the location where the directive is defined.
#
# To iterate through all directives use such code:
# @code
# data = cppcheckdata.parsedump(...)
# for cfg in data.configurations:
#   for directive in cfg.directives:
#     print(directive.str)
# @endcode
#


class Directive:
    ## The directive line, with all C or C++ comments removed
    str = None
    ## name of (possibly included) file where directive is defined
    file = None
    ## line number in (possibly included) file where directive is defined
    linenr = None

    def __init__(self, element):
        self.str = element.get('str')
        self.file = element.get('file')
        self.linenr = element.get('linenr')


## Token class. Contains information about each token in the source code.
#
# The CppcheckData.tokenlist is a list of Token items
#
# C++ class: http://cppcheck.net/devinfo/doxyoutput/classToken.html
#
# To iterate through all tokens use such code:
# @code
# data = cppcheckdata.parsedump(...)
# for cfg in data.configurations:
#   code = ''
#   for token in cfg.tokenlist:
#     code = code + token.str + ' '
#   print(code)
# @endcode
#


class Token:
    Id = None
    ## Token string
    str = None
    ## Next token in tokenlist. For last token, next is None.
    next = None
    ## Previous token in tokenlist. For first token, previous is None,
    previous = None
    linkId = None
    ## Linked token in tokenlist. Each '(', '[' and '{' are linked to the
    # corresponding '}', ']' and ')'. For templates, the '<' is linked to
    # the corresponding '>'.
    link = None
    scopeId = None
    ## Scope information for this token. See the Scope class.
    scope = None
    ## Is this token a symbol name
    isName = False
    ## Is this token a number, for example 123, 12.34
    isNumber = False
    ## Is this token a int value such as 1234
    isInt = False
    ## Is this token a int value such as 12.34
    isFloat = False
    ## Is this token a string literal such as "hello"
    isString = False
    ## string length for string literal
    strlen = None
    ## Is this token a char literal such as 'x'
    isChar = False
    ## Is this token a operator
    isOp = False
    ## Is this token a arithmetic operator
    isArithmeticalOp = False
    ## Is this token a assignment operator
    isAssignmentOp = False
    ## Is this token a comparison operator
    isComparisonOp = False
    ## Is this token a logical operator: && ||
    isLogicalOp = False
    ## varId for token, each variable has a unique non-zero id
    varId = None
    variableId = None
    ## Variable information for this token. See the Variable class.
    #
    # Example code:
    # @code
    # data = cppcheckdata.parsedump(...)
    # code = ''
    # for token in data.tokenlist:
    #   code = code + token.str
    #   if token.variable:
    #     if token.variable.isLocal:
    #       code = code + ':localvar'
    #     if token.variable.isArgument:
    #       code = code + ':arg'
    #   code = code + ' '
    # print(code)
    # @endcode
    variable = None
    functionId = None
    ## If this token points at a function call, this attribute has the Function
    # information. See the Function class.
    function = None
    valuesId = None
    ## Possible values of token
    #
    # Example code:
    # @code
    # data = cppcheckdata.parsedump(...)
    # code = ''
    # for token in data.tokenlist:
    #   code = code + token.str
    #   if token.values:
    #     # print values..
    #     code = code + '{'
    #     for value in token.values:
    #       if value.intvalue:
    #         code = code + str(value.intvalue) + ' '
    #     code = code + '}'
    #   code = code + ' '
    # print(code)
    # @endcode
    values = None

    typeScopeId = None
    ## type scope (token->type()->classScope)
    typeScope = None

    astParentId = None
    ## syntax tree parent
    astParent = None
    astOperand1Id = None
    ## syntax tree operand1
    #
    # Example code:
    # @code
    # data = cppcheckdata.parsedump(...)
    # for token in data.tokenlist:
    #
    #   # is this a addition?
    #   if token.str == '+':
    #
    #     # print LHS operand
    #     print(token.astOperand1.str)
    #
    # @endcode
    astOperand1 = None
    astOperand2Id = None
    ## syntax tree operand2
    #
    # Example code:
    # @code
    # data = cppcheckdata.parsedump(...)
    # for token in data.tokenlist:
    #
    #   # is this a division?
    #   if token.str == '/':
    #
    #     # print RHS operand
    #     print(token.astOperand2.str)
    #
    # @endcode
    astOperand2 = None

    ## file name
    file = None
    ## line number
    linenr = None

    def __init__(self, element):
        self.Id = element.get('id')
        self.str = element.get('str')
        self.next = None
        self.previous = None
        self.scopeId = element.get('scope')
        self.scope = None
        type = element.get('type')
        if type == 'name':
            self.isName = True
        elif type == 'number':
            self.isNumber = True
            if element.get('isInt'):
                self.isInt = True
            elif element.get('isFloat'):
                self.isFloat = True
        elif type == 'string':
            self.isString = True
            self.strlen = int(element.get('strlen'))
        elif type == 'char':
            self.isChar = True
        elif type == 'op':
            self.isOp = True
            if element.get('isArithmeticalOp'):
                self.isArithmeticalOp = True
            elif element.get('isAssignmentOp'):
                self.isAssignmentOp = True
            elif element.get('isComparisonOp'):
                self.isComparisonOp = True
            elif element.get('isLogicalOp'):
                self.isLogicalOp = True
        self.linkId = element.get('link')
        self.link = None
        self.varId = element.get('varId')
        self.variableId = element.get('variable')
        self.variable = None
        self.functionId = element.get('function')
        self.function = None
        self.valuesId = element.get('values')
        self.values = None
        self.typeScopeId = element.get('type-scope')
        self.typeScope = None
        self.astParentId = element.get('astParent')
        self.astParent = None
        self.astOperand1Id = element.get('astOperand1')
        self.astOperand1 = None
        self.astOperand2Id = element.get('astOperand2')
        self.astOperand2 = None
        self.file = element.get('file')
        self.linenr = element.get('linenr')

    def setId(self, IdMap):
        self.scope = IdMap[self.scopeId]
        self.link = IdMap[self.linkId]
        self.variable = IdMap[self.variableId]
        self.function = IdMap[self.functionId]
        self.values = IdMap[self.valuesId]
        self.typeScope = IdMap[self.typeScopeId]
        self.astParent = IdMap[self.astParentId]
        self.astOperand1 = IdMap[self.astOperand1Id]
        self.astOperand2 = IdMap[self.astOperand2Id]

    ## Get value if it exists
    # Returns None if it doesn't exist
    def getValue(self, v):
        if not self.values:
            return None
        for value in self.values:
            if value.intvalue == v:
                return value
        return None

## Scope. Information about global scope, function scopes, class scopes, inner scopes, etc.
# C++ class: http://cppcheck.net/devinfo/doxyoutput/classScope.html


class Scope:
    Id = None
    classStartId = None

    ## The { Token for this scope
    classStart = None
    classEndId = None
    ## The } Token for this scope
    classEnd = None
    ## Name of this scope.
    # For a function scope, this is the function name;
    # for a class scope, this is the class name.
    className = None
    ## Type of scope: Global, Function, Class, If, While
    type = None

    def __init__(self, element):
        self.Id = element.get('id')
        self.className = element.get('className')
        self.classStartId = element.get('classStart')
        self.classStart = None
        self.classEndId = element.get('classEnd')
        self.classEnd = None
        self.nestedInId = element.get('nestedId')
        self.nestedIn = None
        self.type = element.get('type')

    def setId(self, IdMap):
        self.classStart = IdMap[self.classStartId]
        self.classEnd = IdMap[self.classEndId]
        self.nestedIn = IdMap[self.nestedInId]

## Information about a function
# C++ class:
# http://cppcheck.net/devinfo/doxyoutput/classFunction.html


class Function:
    Id = None
    argument = None
    argumentId = None
    tokenDef = None
    tokenDefId = None
    name = None

    def __init__(self, element):
        self.Id = element.get('id')
        self.tokenDefId = element.get('tokenDef')
        self.name = element.get('name')
        self.argument = {}
        self.argumentId = {}
        for arg in element:
            self.argumentId[arg.get('nr')] = arg.get('id')

    def setId(self, IdMap):
        for argnr, argid in self.argumentId.items():
            self.argument[argnr] = IdMap[argid]
        self.tokenDef = IdMap[self.tokenDefId]

## Information about a variable
# C++ class:
# http://cppcheck.net/devinfo/doxyoutput/classVariable.html


class Variable:
    Id = None
    nameTokenId = None
    ## name token in variable declaration
    nameToken = None
    typeStartTokenId = None
    ## start token of variable declaration
    typeStartToken = None
    typeEndTokenId = None
    ## end token of variable declaration
    typeEndToken = None
    ## Is this variable a function argument?
    isArgument = False
    ## Is this variable an array?
    isArray = False
    ## Is this variable a class or struct?
    isClass = False
    ## Is this variable a local variable?
    isLocal = False
    ## Is this variable a pointer
    isPointer = False
    ## Is this variable a reference
    isReference = False
    ## Is this variable static?
    isStatic = False

    def __init__(self, element):
        self.Id = element.get('id')
        self.nameTokenId = element.get('nameToken')
        self.nameToken = None
        self.typeStartTokenId = element.get('typeStartToken')
        self.typeStartToken = None
        self.typeEndTokenId = element.get('typeEndToken')
        self.typeEndToken = None
        self.isArgument = element.get('isArgument') == 'true'
        self.isArray = element.get('isArray') == 'true'
        self.isClass = element.get('isClass') == 'true'
        self.isLocal = element.get('isLocal') == 'true'
        self.isPointer = element.get('isPointer') == 'true'
        self.isReference = element.get('isReference') == 'true'
        self.isStatic = element.get('isStatic') == 'true'

    def setId(self, IdMap):
        self.nameToken = IdMap[self.nameTokenId]
        self.typeStartToken = IdMap[self.typeStartTokenId]
        self.typeEndToken = IdMap[self.typeEndTokenId]

## ValueFlow class


class ValueFlow:
    ## ValueFlow::Value class
    # Each possible value has a ValueFlow::Value item.
    # Each ValueFlow::Value either has a intvalue or tokvalue
    # C++ class:
    # http://cppcheck.net/devinfo/doxyoutput/classValueFlow_1_1Value.html

    class Value:
        ## integer value
        intvalue = None
        ## token value
        tokvalue = None
        ## condition where this Value comes from
        condition = None

        def __init__(self, element):
            self.intvalue = element.get('intvalue')
            if self.intvalue:
                self.intvalue = int(self.intvalue)
            self.tokvalue = element.get('tokvalue')
            self.condition = element.get('condition-line')
            if self.condition:
                self.condition = int(self.condition)

    Id = None

    ## Possible values
    values = None

    def __init__(self, element):
        self.Id = element.get('id')
        self.values = []
        for value in element:
            self.values.append(ValueFlow.Value(value))

## Configuration class
# This class contains the directives, tokens, scopes, functions,
# variables and value flows for one configuration.


class Configuration:
    ## Name of the configuration, "" for default
    name = ''
    ## List of Directive items
    directives = []
    ## List of Token items
    tokenlist = []
    ## List of Scope items
    scopes = []
    ## List of Function items
    functions = []
    ## List of Variable items
    variables = []
    ## List of ValueFlow values
    valueflow = []

    def __init__(self, confignode):
        self.name = confignode.get('cfg')
        self.directives = []
        self.tokenlist = []
        self.scopes = []
        self.functions = []
        self.variables = []
        self.valueflow = []

        for element in confignode:
            if element.tag == 'directivelist':
                for directive in element:
                    self.directives.append(Directive(directive))

            if element.tag == 'tokenlist':
                for token in element:
                    self.tokenlist.append(Token(token))

                # set next/previous..
                prev = None
                for token in self.tokenlist:
                    token.previous = prev
                    if prev:
                        prev.next = token
                    prev = token
            if element.tag == 'scopes':
                for scope in element:
                    self.scopes.append(Scope(scope))
                    for functionList in scope:
                        if functionList.tag == 'functionList':
                            for function in functionList:
                                self.functions.append(Function(function))
            if element.tag == 'variables':
                for variable in element:
                    self.variables.append(Variable(variable))
            if element.tag == 'valueflow':
                for values in element:
                    self.valueflow.append(ValueFlow(values))

        IdMap = {}
        IdMap[None] = None
        IdMap['0'] = None
        for token in self.tokenlist:
            IdMap[token.Id] = token
        for scope in self.scopes:
            IdMap[scope.Id] = scope
        for function in self.functions:
            IdMap[function.Id] = function
        for variable in self.variables:
            IdMap[variable.Id] = variable
        for values in self.valueflow:
            IdMap[values.Id] = values.values

        for token in self.tokenlist:
            token.setId(IdMap)
        for scope in self.scopes:
            scope.setId(IdMap)
        for function in self.functions:
            function.setId(IdMap)
        for variable in self.variables:
            variable.setId(IdMap)

## Class that makes cppcheck dump data available
# Contains a list of Configuration instances
#
# To iterate through all configurations use such code:
# @code
# data = cppcheckdata.parsedump(...)
# for cfg in data.configurations:
#     print('cfg: ' + cfg.name)
# @endcode
#
# To iterate through all tokens in each configuration use such code:
# @code
# data = cppcheckdata.parsedump(...)
# for cfg in data.configurations:
#     print('cfg: ' + cfg.name)
#     code = ''
#         for token in cfg.tokenlist:
#             code = code + token.str + ' '
#     print('    ' + code)
# @endcode
#
# To iterate through all scopes (functions, types, etc) use such code:
# @code
# data = cppcheckdata.parsedump(...)
# for cfg in data.configurations:
#     print('cfg: ' + cfg.name)
#     for scope in cfg.scopes:
#         print('    type:' + scope.type + ' name:' + scope.className)
# @endcode
#
#


class CppcheckData:
    ## List of Configurations
    configurations = []

    def __init__(self, filename):
        self.configurations = []

        data = ET.parse(filename)
        # root is 'dumps' node, each config has its own 'dump' subnode.
        for cfgnode in data.getroot():
            self.configurations.append(Configuration(cfgnode))

## parse a cppcheck dump file


def parsedump(filename):
    return CppcheckData(filename)

## Check if type of ast node is float/double


def astIsFloat(token):
    if not token:
        return False
    if token.str == '.':
        return astIsFloat(token.astOperand2)
    if '+-*/%'.find(token.str) == 0:
        if True == astIsFloat(token.astOperand1):
            return True
        return astIsFloat(token.astOperand2)
    if not token.variable:
        # float literal?
        if token.str[0].isdigit():
            for c in token.str:
                if c == 'f' or c == '.' or c == 'E':
                    return True
        return False
    typeToken = token.variable.typeStartToken
    endToken = token.variable.typeEndToken
    while typeToken != endToken:
        if typeToken.str == 'float' or typeToken.str == 'double':
            return True
        typeToken = typeToken.next
    if typeToken.str == 'float' or typeToken.str == 'double':
        return True
    return False

# Create a cppcheck parser

class CppCheckFormatter(argparse.HelpFormatter):
    '''
        Properly formats multiline argument helps
    '''
    def _split_lines(self, text, width):
        # this is the RawTextHelpFormatter._split_lines
        if text.startswith('R|'):
            return text[2:].splitlines()  
        return argparse.HelpFormatter._split_lines(self, text, width)

def ArgumentParser():
    '''
        Returns an argparse argument parser with an already-added
        argument definition for -t/--template
    '''
    parser = argparse.ArgumentParser(formatter_class=CppCheckFormatter)
    parser.add_argument('-t', '--template', metavar='<text>',
        default='{callstack}: ({severity}) {message}',
        help="R|Format the error messages. E.g.\n" \
             "'{file}:{line},{severity},{id},{message}' or\n" \
             "'{file}({line}):({severity}) {message}' or\n" \
             "'{callstack} {message}'\n" \
             "Pre-defined templates: gcc, vs, edit")
    return parser

# Format an error message.
 
def reportError(template, callstack=[], severity='', message='', id=''):
    '''
        Format an error message according to the template.
        
        :param template: format string, or 'gcc', 'vs' or 'edit'.
        :param callstack: e.g. [['file1.cpp',10],['file2.h','20'], ... ]
        :param severity: e.g. 'error', 'warning' ...
        :param id: message ID.
        :param message: message text.
    '''
    # expand predefined templates
    if template == 'gcc':
        template = '{file}:{line}: {severity}: {message}'
    elif template == 'vs':
        template = '{file}({line}): {severity}: {message}'
    elif template == 'edit':
        template = '{file} +{line}: {severity}: {message}'
    # compute 'callstack}, {file} and {line} replacements
    stack = ' -> '.join(['['+f+':'+str(l)+']' for (f,l) in callstack])
    file = callstack[-1][0]
    line = str(callstack[-1][1])
    # format message
    return template.format(callstack=stack, file=file, line=line,
        severity=severity, message=message, id=id)
