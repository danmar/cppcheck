"""
cppcheckdata

This is a Python module that helps you access Cppcheck dump data.

License: No restrictions, use this as you need.
"""

import xml.etree.ElementTree as ET
import argparse


class Directive:
    """
    Directive class. Contains information about each preprocessor directive in the source code.

    Attributes:
        str      The directive line, with all C or C++ comments removed
        file     Name of (possibly included) file where directive is defined
        linenr   Line number in (possibly included) file where directive is defined

    To iterate through all directives use such code:
    @code
    data = cppcheckdata.parsedump(...)
    for cfg in data.configurations:
      for directive in cfg.directives:
        print(directive.str)
    @endcode
    """

    str = None
    file = None
    linenr = None

    def __init__(self, element):
        self.str = element.get('str')
        self.file = element.get('file')
        self.linenr = element.get('linenr')


class Token:
    """
    Token class. Contains information about each token in the source code.

    The CppcheckData.tokenlist is a list of Token items

    C++ class: http://cppcheck.net/devinfo/doxyoutput/classToken.html

    Attributes:
        str                Token string
        next               Next token in tokenlist. For last token, next is None.
        previous           Previous token in tokenlist. For first token, previous is None.
        link               Linked token in tokenlist. Each '(', '[' and '{' are linked to the
                           corresponding '}', ']' and ')'. For templates, the '<' is linked to
                           the corresponding '>'.
        scope              Scope information for this token. See the Scope class.
        isName             Is this token a symbol name
        isNumber           Is this token a number, for example 123, 12.34
        isInt              Is this token a int value such as 1234
        isFloat            Is this token a int value such as 12.34
        isString           Is this token a string literal such as "hello"
        strlen             string length for string literal
        isChar             Is this token a char literal such as 'x'
        isOp               Is this token a operator
        isArithmeticalOp   Is this token a arithmetic operator
        isAssignmentOp     Is this token a assignment operator
        isComparisonOp     Is this token a comparison operator
        isLogicalOp        Is this token a logical operator: && ||
        isUnsigned         Is this token a unsigned type
        isSigned           Is this token a signed type
        varId              varId for token, each variable has a unique non-zero id
        variable           Variable information for this token. See the Variable class.
        function           If this token points at a function call, this attribute has the Function
                           information. See the Function class.
        values             Possible values of token
        typeScope          type scope (token->type()->classScope)
        astParent          ast parent
        astOperand1        ast operand1
        astOperand2        ast operand2
        file               file name
        linenr             line number

    To iterate through all tokens use such code:
    @code
    data = cppcheckdata.parsedump(...)
    for cfg in data.configurations:
      code = ''
      for token in cfg.tokenlist:
        code = code + token.str + ' '
      print(code)
    @endcode
    """


    Id = None
    str = None
    next = None
    previous = None
    linkId = None
    link = None
    scopeId = None
    scope = None
    isName = False
    isNumber = False
    isInt = False
    isFloat = False
    isString = False
    strlen = None
    isChar = False
    isOp = False
    isArithmeticalOp = False
    isAssignmentOp = False
    isComparisonOp = False
    isLogicalOp = False
    isUnsigned = False
    isSigned = False
    varId = None
    variableId = None
    variable = None
    functionId = None
    function = None
    valuesId = None
    values = None

    typeScopeId = None
    typeScope = None

    astParentId = None
    astParent = None
    astOperand1Id = None
    astOperand1 = None
    astOperand2Id = None
    astOperand2 = None

    file = None
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
            if element.get('isUnsigned'):
                self.isUnsigned = True
            if element.get('isSigned'):
                self.isSigned = True
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

    def getValue(self, v):
        """
        Get value if it exists
        Returns None if it doesn't exist
        """

        if not self.values:
            return None
        for value in self.values:
            if value.intvalue == v:
                return value
        return None


class Scope:
    """
    Scope. Information about global scope, function scopes, class scopes, inner scopes, etc.
    C++ class: http://cppcheck.net/devinfo/doxyoutput/classScope.html

    Attributes
        classStart     The { Token for this scope
        classEnd       The } Token for this scope
        className      Name of this scope.
                       For a function scope, this is the function name;
                       For a class scope, this is the class name.
        type           Type of scope: Global, Function, Class, If, While
    """

    Id = None
    classStartId = None

    classStart = None
    classEndId = None
    classEnd = None
    className = None
    type = None

    def __init__(self, element):
        self.Id = element.get('id')
        self.className = element.get('className')
        self.classStartId = element.get('classStart')
        self.classStart = None
        self.classEndId = element.get('classEnd')
        self.classEnd = None
        self.nestedInId = element.get('nestedIn')
        self.nestedIn = None
        self.type = element.get('type')

    def setId(self, IdMap):
        self.classStart = IdMap[self.classStartId]
        self.classEnd = IdMap[self.classEndId]
        self.nestedIn = IdMap[self.nestedInId]



class Function:
    """
    Information about a function
    C++ class:
    http://cppcheck.net/devinfo/doxyoutput/classFunction.html
    """

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


class Variable:
    """
    Information about a variable
    C++ class:
    http://cppcheck.net/devinfo/doxyoutput/classVariable.html

    Attributes:
        nameToken       name token in variable declaration
        typeStartToken  start token of variable declaration
        typeEndToken    end token of variable declaration
        isArgument      Is this variable a function argument?
        isArray         Is this variable an array?
        isClass         Is this variable a class or struct?
        isLocal         Is this variable a local variable?
        isPointer       Is this variable a pointer
        isReference     Is this variable a reference
        isStatic        Is this variable static?
    """

    Id = None
    nameTokenId = None
    nameToken = None
    typeStartTokenId = None
    typeStartToken = None
    typeEndTokenId = None
    typeEndToken = None
    isArgument = False
    isArray = False
    isClass = False
    isLocal = False
    isPointer = False
    isReference = False
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



class ValueFlow:
    """
    ValueFlow::Value class
    Each possible value has a ValueFlow::Value item.
    Each ValueFlow::Value either has a intvalue or tokvalue
    C++ class:
    http://cppcheck.net/devinfo/doxyoutput/classValueFlow_1_1Value.html

    Attributes:
        values    Possible values
    """


    Id = None
    values = None

    class Value:
        """
        Value class

        Attributes:
            intvalue     integer value
            tokvalue     token value
            condition    condition where this Value comes from
        """

        intvalue = None
        tokvalue = None
        condition = None

        def __init__(self, element):
            self.intvalue = element.get('intvalue')
            if self.intvalue:
                self.intvalue = int(self.intvalue)
            self.tokvalue = element.get('tokvalue')
            self.condition = element.get('condition-line')
            if self.condition:
                self.condition = int(self.condition)

    def __init__(self, element):
        self.Id = element.get('id')
        self.values = []
        for value in element:
            self.values.append(ValueFlow.Value(value))



class Configuration:
    """
    Configuration class
    This class contains the directives, tokens, scopes, functions,
    variables and value flows for one configuration.

    Attributes:
        name          Name of the configuration, "" for default
        directives    List of Directive items
        tokenlist     List of Token items
        scopes        List of Scope items
        functions     List of Function items
        variables     List of Variable items
        valueflow     List of ValueFlow values
    """

    name = ''
    directives = []
    tokenlist = []
    scopes = []
    functions = []
    variables = []
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



class CppcheckData:
    """
    Class that makes cppcheck dump data available
    Contains a list of Configuration instances

    Attributes:
        configurations    List of Configurations

    To iterate through all configurations use such code:
    @code
    data = cppcheckdata.parsedump(...)
    for cfg in data.configurations:
        print('cfg: ' + cfg.name)
    @endcode

    To iterate through all tokens in each configuration use such code:
    @code
    data = cppcheckdata.parsedump(...)
    for cfg in data.configurations:
        print('cfg: ' + cfg.name)
        code = ''
            for token in cfg.tokenlist:
                code = code + token.str + ' '
        print('    ' + code)
    @endcode

    To iterate through all scopes (functions, types, etc) use such code:
    @code
    data = cppcheckdata.parsedump(...)
    for cfg in data.configurations:
        print('cfg: ' + cfg.name)
        for scope in cfg.scopes:
            print('    type:' + scope.type + ' name:' + scope.className)
    @endcode
    """

    configurations = []

    def __init__(self, filename):
        self.configurations = []

        data = ET.parse(filename)
        # root is 'dumps' node, each config has its own 'dump' subnode.
        for cfgnode in data.getroot():
            self.configurations.append(Configuration(cfgnode))



def parsedump(filename):
    """
    parse a cppcheck dump file
    """
    return CppcheckData(filename)



def astIsFloat(token):
    """
    Check if type of ast node is float/double
    """

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


class CppCheckFormatter(argparse.HelpFormatter):
    """
    Properly formats multiline argument helps
    """
    def _split_lines(self, text, width):
        # this is the RawTextHelpFormatter._split_lines
        if text.startswith('R|'):
            return text[2:].splitlines()
        return argparse.HelpFormatter._split_lines(self, text, width)

def ArgumentParser():
    """
        Returns an argparse argument parser with an already-added
        argument definition for -t/--template
    """
    parser = argparse.ArgumentParser(formatter_class=CppCheckFormatter)
    parser.add_argument('-t', '--template', metavar='<text>',
                        default='{callstack}: ({severity}) {message}',
                        help="R|Format the error messages. E.g.\n"
                        "'{file}:{line},{severity},{id},{message}' or\n"
                        "'{file}({line}):({severity}) {message}' or\n"
                        "'{callstack} {message}'\n"
                        "Pre-defined templates: gcc, vs, edit")
    return parser

def reportError(template, callstack=[], severity='', message='', id=''):
    """
        Format an error message according to the template.

        :param template: format string, or 'gcc', 'vs' or 'edit'.
        :param callstack: e.g. [['file1.cpp',10],['file2.h','20'], ... ]
        :param severity: e.g. 'error', 'warning' ...
        :param id: message ID.
        :param message: message text.
    """
    # expand predefined templates
    if template == 'gcc':
        template = '{file}:{line}: {severity}: {message}'
    elif template == 'vs':
        template = '{file}({line}): {severity}: {message}'
    elif template == 'edit':
        template = '{file} +{line}: {severity}: {message}'
    # compute 'callstack}, {file} and {line} replacements
    stack = ' -> '.join(['[' + f + ':' + str(l) + ']' for (f, l) in callstack])
    file = callstack[-1][0]
    line = str(callstack[-1][1])
    # format message
    return template.format(callstack=stack, file=file, line=line,
                           severity=severity, message=message, id=id)
