# Python module that loads a cppcheck dump
# License: No restrictions, use this as you need.

from lxml import etree


class Token:
    Id = None
    str = None
    next = None
    previous = None
    scopeId = None
    scope = None
    isName = None
    isNumber = None
    isInt = None
    isFloat = None
    isString = None
    strlen = None
    isChar = None
    isOp = None
    isArithmeticalOp = None
    isAssignmentOp = None
    isComparisonOp = None
    isLogicalOp = None
    linkId = None
    link = None
    varId = None
    variableId = None
    variable = None
    functionId = None
    function = None
    valuesId = None
    values = None
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
        self.astParent = IdMap[self.astParentId]
        self.astOperand1 = IdMap[self.astOperand1Id]
        self.astOperand2 = IdMap[self.astOperand2Id]

    # Get value if it exists
    # Returns None if it doesn't exist
    def getValue(self, v):
        if not self.values:
            return None
        for value in self.values:
            if value.intvalue == v:
                return value
        return None


class Scope:
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
        self.nestedInId = element.get('nestedId')
        self.nestedIn = None
        self.type = element.get('type')

    def setId(self, IdMap):
        self.classStart = IdMap[self.classStartId]
        self.classEnd = IdMap[self.classEndId]
        self.nestedIn = IdMap[self.nestedInId]


class Function:
    Id = None
    argument = None
    argumentId = None

    def __init__(self, element):
        self.Id = element.get('id')
        self.argument = {}
        self.argumentId = {}
        for arg in element:
            self.argumentId[arg.get('nr')] = arg.get('id')

    def setId(self, IdMap):
        for argnr, argid in self.argumentId.items():
            self.argument[argnr] = IdMap[argid]


class Variable:
    Id = None
    nameTokenId = None
    nameToken = None
    typeStartTokenId = None
    typeStartToken = None
    typeEndTokenId = None
    typeEndToken = None
    isArgument = None
    isArray = None
    isClass = None
    isLocal = None
    isPointer = None
    isReference = None
    isStatic = None

    def __init__(self, element):
        self.Id = element.get('id')
        self.nameTokenId = element.get('nameToken')
        self.nameToken = None
        self.typeStartTokenId = element.get('typeStartToken')
        self.typeStartToken = None
        self.typeEndTokenId = element.get('typeEndToken')
        self.typeEndToken = None
        self.isArgument = element.get('isArgument')
        self.isArray = element.get('isArray')
        self.isClass = element.get('isClass')
        self.isLocal = element.get('isLocal')
        self.isPointer = element.get('isPointer')
        self.isReference = element.get('isReference')
        self.isStatic = element.get('isStatic')

    def setId(self, IdMap):
        self.nameToken = IdMap[self.nameTokenId]
        self.typeStartToken = IdMap[self.typeStartTokenId]
        self.typeEndToken = IdMap[self.typeEndTokenId]


class ValueFlow:

    class Value:
        intvalue = None
        condition = None

        def __init__(self, element):
            self.intvalue = int(element.get('intvalue'))
            self.condition = element.get('condition-line')
            if self.condition:
                self.condition = int(self.condition)

    Id = None
    values = None

    def __init__(self, element):
        self.Id = element.get('id')
        self.values = []
        for value in element:
            self.values.append(ValueFlow.Value(value))


class CppcheckData:
    tokenlist = []
    scopes = []
    functions = []
    variables = []
    valueflow = []

    def __init__(self, filename):
        self.tokenlist = []
        self.scopes = []
        self.variables = []
        self.valueflow = []

        data = etree.parse(filename)
        for element in data.getroot():
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


def parsedump(filename):
    return CppcheckData(filename)

# Check if type of ast node is float/double


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
