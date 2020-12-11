# Holds information about a struct or union's element definition.
class ElementDef:
    def __init__(self, elementType, name, valueType, dimensions = None):
        self.elementType = elementType
        self.name = str(name)
        self.valueType = valueType
        self.children = []
        self.dimensions = dimensions
        self.parent = None

        self.isDesignated = False
        self.isPositional = False
        self.numInits = 0
        self.childIndex = -1

    def __repr__(self):
        inits = ""
        if self.isPositional:
            inits += 'P'
        if self.isDesignated:
            inits += 'D'
        if not (self.isPositional or self.isDesignated) and self.numInits == 0:
            inits += '_'
        if (self.numInits > 1):
            inits += str(self.numInits)

        attrs = ["childIndex", "elementType", "valueType"]
        return "{}({}, {}, {})".format(
            "ED",
            self.getLongName(),
            inits,
            ", ".join(("{}={}".format(a, repr(getattr(self, a))) for a in attrs))
        )

    def getLongName(self):
        return self.parent.getLongName() + "." + self.name if self.parent else self.name

    def getDebugDump(self, level = 0):
        t = []
        if self.isPositional:
            t.append('P')
        if self.isDesignated:
            t.append('D')
        if self.numInits == 0:
            t.append('_')
        if (self.numInits > 1):
            t.append(str(self.numInits))
        myDump = self.name + ":" + "/".join(t)

        if len(self.children):
            childDumps = []
            for c in self.children:
                childDumps.append(c.getDebugDump(level + 1))
            myDump += "{\n" + '  ' * (level + 1) + ", ".join(childDumps) + "\n" + '  ' * level + "}"

        return myDump

    def getSlimDump(self):
        t = []
        if self.isPositional:
            t.append('P')
        if self.isDesignated:
            t.append('D')
        if self.numInits == 0:
            t.append('_')
        if (self.numInits > 1):
            t.append(str(self.numInits))

        myDump = "".join(t)

        if len(self.children):
            childDumps = []
            for c in self.children:
                childDumps.append(c.getSlimDump())
            myDump += "{ " + ", ".join(childDumps) + " }"

        return myDump


    def addChild(self, child):
        self.children.append(child)
        child.parent = self

    def getNextChild(self):
        self.childIndex += 1
        return self.getChildByIndex(self.childIndex)

    def getChildByIndex(self, index):
        return self.children[index] if index >= 0 and len(self.children) > index else None

    def getChildByName(self, name):
        for c in self.children:
            if c.name == name:
                return c
        return None

    def setInitialized(self, designated=False, positional=False):
        if designated:
            self.isDesignated = True
        if positional or not designated:
            self.isPositional = True
        self.numInits += 1

    def initializeChildren(self):
        for child in self.children:
            child.setInitialized(positional=True)
            child.initializeChildren()

    def unset(self):
        self.isDesignated = False
        self.isPositional = False

        # Unset is always recursive
        for child in self.children:
            child.unset()

    def isAllChildrenSet(self):
        myself = len(self.children) == 0 and (self.isDesignated or self.isPositional)
        mychildren =  len(self.children) > 0 and all([child.isAllChildrenSet() for child in self.children])
        return myself or mychildren

    def isAllSet(self):
        return all([child.isPositional or child.isDesignated for child in self.children])

    def isOnlyDesignated(self):
        return all([not child.isPositional for child in self.children])

    def isMisra93Compliant(self):
        if self.elementType == 'array':
            result = self.isAllChildrenSet() or \
                ((self.isAllSet() or \
                self.isOnlyDesignated()) and \
                all([not (child.isDesignated or child.isPositional) or child.isMisra93Compliant() for child in self.children]))
            return result
        elif self.elementType == 'record':
            result = all([child.isMisra93Compliant() for child in self.children])
            return result
        else:
            return True

    def isMisra94Compliant(self):
        return self.numInits <= 1 and all([child.isMisra94Compliant() for child in self.children])

    def markAsCurrent(self):
        if self.parent:
            if self.name == '<-':
                self.parent.childIndex = self.parent.children.index(self.children[0])
            else:
                self.parent.childIndex = self.parent.children.index(self)

            self.parent.markAsCurrent()

    def getNextValueElement(self, root):
        current = self
        while current != root:
            # Get next index of parent
            i = current.parent.children.index(current) + 1
            # Next index of parent exists
            if i < len(current.parent.children):
                current = current.parent.children[i]
                return current.getFirstValueElement()

            # Next index of parent doesn't exist. Move up
            current = current.parent
        return None

    def getFirstValueElement(self):
        current = self
        # Move to first child as long as children exists
        while len(current.children) > 0:
            current = current.children[0]
        return current

    def getLastValueElement(self):
        current = self
        # Move to last child as long as children exists
        while len(current.children) > 0:
            current = current.children[-1]
        return current

    def getEffectiveLevel(self):
        if self.parent and self.parent.elementType == "array":
            return self.parent.getEffectiveLevel() + 1
        else:
            return 0

    @property
    def isArray(self):
        return self.elementType == 'array'

    @property
    def isRecord(self):
        return self.elementType == 'record'

    @property
    def isValue(self):
        return self.elementType == 'value'


def misra_9_x(self, data, rule):
    for variable in data.variables:
        if not variable.nameToken:
            continue

        nameToken = variable.nameToken

        # Check if declaration and initialization is
        # split into two separate statements in ast.
        if nameToken.next and nameToken.next.isSplittedVarDeclEq:
            nameToken = nameToken.next.next

        # Find declarations with initializer assignment
        eq = nameToken
        while not eq.isAssignmentOp and eq.astParent:
            eq = eq.astParent

        # We are only looking for initializers
        if not eq.isAssignmentOp or eq.astOperand2.isName:
            continue

        if rule == 902:
            if variable.isArray :
                dimensions, valueType = getArrayDimensionsAndValueType(eq.astOperand1)
                if dimensions is None:
                    continue

                checkArrayInitializer(self, eq.astOperand2, dimensions, valueType)
            elif variable.isClass:
                if not nameToken.valueType:
                    continue

                valueType = nameToken.valueType
                if valueType.type == 'record':
                    elements = getRecordElements(valueType)
                    checkObjectInitializer(self, eq.astOperand2, elements)

        else:
            if variable.isArray or variable.isClass:
                ed = getElementDef(nameToken)
                parseInitializer(ed, eq.astOperand2, rule)
                # print(rule, nameToken.str + '=', ed.getSlimDump())
                if rule == 903 and not ed.isMisra93Compliant():
                    self.reportError(nameToken, 9, 3)
                if rule == 904 and not ed.isMisra94Compliant():
                    self.reportError(nameToken, 9, 4)


def getElementDef(nameToken):
    if nameToken.variable.isArray:
        ed = ElementDef("array", nameToken.str, nameToken.valueType)
        createArrayChildrenDefs(ed, nameToken.astParent)
    elif nameToken.variable.isClass:
        ed = ElementDef("record", nameToken.str, nameToken.valueType)
        createRecordChildrenDefs(ed)
    else:
        ed = ElementDef("value", nameToken.str, nameToken.valueType)
    return ed

def createArrayChildrenDefs(ed, token):
    if token.str == '[':
        if token.astOperand2 is not None:
            for i in range(token.astOperand2.getKnownIntValue()):
                if token.astParent and token.astParent.str == '[':
                    child = ElementDef("array", i, ed.valueType)
                    createArrayChildrenDefs(child, token.astParent)
                else:
                    if ed.valueType.type == "record":
                        child = ElementDef("record", i, ed.valueType)
                        createRecordChildrenDefs(child)
                    else:
                        child = ElementDef("value", i, ed.valueType)

                ed.addChild(child)

def createRecordChildrenDefs(ed):
    valueType = ed.valueType
    if not valueType or not valueType.typeScope:
        return

    for variable in valueType.typeScope.varlist:
        child = getElementDef(variable.nameToken)
        ed.addChild(child)

def getElementByDesignator(ed, token):
    while token.str in [ '.', '[' ]:
        token = token.astOperand1

    while ed and not token.isAssignmentOp:
        token = token.astParent

        if token.str == '[':
            chIndex = -1
            if token.astOperand2 is not None:
                chIndex = token.astOperand2.getKnownIntValue()
            elif token.astOperand1 is not None:
                chIndex = token.astOperand1.getKnownIntValue()

            ed = ed.getChildByIndex(chIndex)

        elif token.str == '.':
            name = ""
            if token.astOperand2 is not None:
                name = token.astOperand2.str
            elif token.astOperand1 is not None:
                name = token.astOperand1.str

            ed = ed.getChildByName(name)

    return ed

def parseInitializer(root, token, rule):
    def pushToRootStackAndMarkAsDesignated(old, nextEd):
        new = nextEd.parent
        if new != old:
            # Mark all elements up to old root as designated
            parent = new
            while parent != old:
                parent.isDesignated = True
                parent = parent.parent
            rootStack.append((old, new))
            new.markAsCurrent()
        new.childIndex = new.children.index(nextEd) - 1
        return new

    def popFromStackIfExitElement(element):
        if len(rootStack) > 0 and rootStack[-1][1] == element:
            old = rootStack.pop()[0]
            old.markAsCurrent()
            return old
        return element

    def unwindAndContinue():
        nonlocal token, root, ed
        while token:
            if token.astParent.astOperand1 == token and token.astParent.astOperand2:
                if ed:
                    ed.markAsCurrent()
                    ed = ed.getNextValueElement(root)

                token = token.astParent.astOperand2
                break
            else:
                token = token.astParent
                if token.str == '{':
                    ed = root.getLastValueElement()
                    ed.markAsCurrent()

                    # Cleanup if root is dummy node representing excess levels in initializer
                    if root and root.name == '<-':
                        root.children[0].parent = root.parent

                    root = root.parent

                if token.astParent == None:
                    token = None
                    break

    if not token.str == '{':
        # 9.2
        return

    rootStack = []

    token = token.astOperand1
    isFirstElement = True
    isDesignated = False

    ed = root.getFirstValueElement()

    while token:
        if token.str == ',':
            token = token.astOperand1
            isFirstElement = False

        # Designated initializer ( [2]=...  or .name=... )
        elif token.isAssignmentOp and not token.valueType:
            root = popFromStackIfExitElement(root)

            ed = getElementByDesignator(root, token.astOperand1)
            if ed:
                root = pushToRootStackAndMarkAsDesignated(root, ed)

            token = token.astOperand2
            isFirstElement = False
            isDesignated = True

        elif token.str == '{':
            nextChild = root.getNextChild()
            if nextChild:
                if nextChild.isArray or nextChild.isRecord:
                    nextChild.unset()
                    nextChild.setInitialized(isDesignated)
                    ed = nextChild.getFirstValueElement()
                    isDesignated = False
                elif token.astOperand1:
                    # Create dummy nextChild to represent excess levels in initializer
                    dummyRoot = ElementDef('array', '<-', root.valueType)
                    dummyRoot.parent = root
                    dummyRoot.childIndex = 0
                    dummyRoot.children = [nextChild]
                    nextChild.parent = dummyRoot

                    # Fake dummy as nextChild (of current root)
                    nextChild = dummyRoot

            if token.astOperand1:
                root = nextChild
                token = token.astOperand1
                isFirstElement = True
            else:
                # {}
                ed = None
                unwindAndContinue()

        else:
            if ed and ed.isValue:
                if isFirstElement and token.str == '0' and token.next.str == '}':
                    # Zero initializer causes recursive initialization
                    root.initializeChildren()
                elif  token.isString and ed.valueType.pointer > 0:
                    if ed.valueType.pointer - ed.getEffectiveLevel() == 1:
                        ed.setInitialized(isDesignated)
                    elif ed.valueType.pointer == ed.getEffectiveLevel():
                        ed.parent.setInitialized(isDesignated)
                        ed.parent.initializeChildren()
                else:
                    ed.setInitialized(isDesignated)

                # Mark all elements up to root with positional or designated
                # (for complex designators, or missing structure)
                parent = ed.parent
                while parent and parent != root:
                    parent.isDesignated = isDesignated if isDesignated else parent.isDesignated
                    parent.isPositional = not isDesignated if not isDesignated else parent.isPositional
                    parent = parent.parent
                isDesignated = False

            unwindAndContinue()



# Return an array containing the size of each dimension of an array declaration,
# or coordinates of a designator in an array initializer,
# and the name token's valueType, if it exist.
#
# In the examples below, the ^ indicates the initial token passed to the function.
#
# Ex:   int arr[1][2][3] = .....
#                    ^
#       returns: [1,2,3], valueType
#
# Ex:   int arr[3][4] = { [1][2] = 5 }
#                            ^
#       returns [1,2], None
def getArrayDimensionsAndValueType(token):
    dimensions = []

    while token.str == '*':
        if token.astOperand2 is not None:
            token = token.astOperand2
        else:
            token = token.astOperand1

    # while token.astOperand1.str == '[':
    #     token = token.astOperand1

    while token and token.str == '[':
        if token.astOperand2 is not None:
            dimensions.insert(0, token.astOperand2.getKnownIntValue())
            token = token.astOperand1
        elif token.astOperand1 is not None:
            dimensions.insert(0, token.astOperand1.getKnownIntValue())
            break
        else:
            dimensions = None
            break

    valueType = token.valueType if token else None
    name = token.str if token else None

    return dimensions, valueType

    # subDimensions = dimensions
    # for d, i in dimensions:
    #     for j in range(d):
    #         e = ElementDef('array', name, valueType, )

    # root = ElementDef('array', name, valueType, children)



# Returns a list of the struct elements as StructElementDef in the order they are declared.
def getRecordElements(valueType):
    if not valueType or not valueType.typeScope:
        return []

    elements = []
    for variable in valueType.typeScope.varlist:
        if variable.isArray:
            dimensions, arrayValueType = getArrayDimensionsAndValueType(variable.nameToken.astParent)
            elements.append(ElementDef('array', variable.nameToken.str, arrayValueType, dimensions))
        elif variable.isClass:
            elements.append(ElementDef('record', variable.nameToken.str, variable.nameToken.valueType))
        else:
            elements.append(ElementDef('value', variable.nameToken.str, variable.nameToken.valueType))

    return elements

# Checks if the initializer conforms to the dimensions of the array declaration
# at a given level.
# Parameters:
#   token:      root node of the initializer tree
#   dimensions: dimension sizes of the array declaration
#   valueType:  the array type
def checkArrayInitializer(self, token, dimensions, valueType):
    level = 0
    levelOffsets = [] # Calculated when designators in initializers are used
    elements = getRecordElements(valueType) if valueType.type == 'record' else None

    isFirstElement = False
    while token:
        if token.str == ',':
            token = token.astOperand1
            isFirstElement = False
            continue

        if token.isAssignmentOp and not token.valueType:
            designator, _ = getArrayDimensionsAndValueType(token.astOperand1)
            # Calculate level offset based on designator in initializer
            levelOffsets[-1] = len(designator) - 1
            token = token.astOperand2
            isFirstElement = False

        effectiveLevel = sum(levelOffsets) + level

        # Zero initializer is ok at any level
        isZeroInitializer = (isFirstElement and token.str == '0')
        # String initializer is ok at one level below value level unless array to pointers
        isStringInitializer = token.isString and effectiveLevel == len(dimensions) - 1 and valueType.pointer == len(dimensions)

        if effectiveLevel == len(dimensions) or isZeroInitializer or isStringInitializer:
            if not isZeroInitializer and not isStringInitializer:
                isFirstElement = False
                if valueType.type == 'record':
                    if token.isName:
                        if not token.valueType.typeScope  == valueType.typeScope:
                            self.reportError(token, 9, 2)
                            return False
                    else:
                        if not checkObjectInitializer(self, token, elements):
                            return False
                elif token.str == '{':
                    self.reportError(token, 9, 2)
                    return False
                # String initializer is not ok at this level, unless array to pointers
                # (should be pointer to const-qualified char, but that check is out of scope for 9.2)
                elif token.isString and valueType.pointer == len(dimensions):
                    self.reportError(token, 9, 2)
                    return False

            # Done evaluating leaf node - go back up to find next astOperand2
            while token:
                # Done checking once level is back to 0 (or we run out of parents)
                if level == 0 or not token.astParent:
                    return True

                if  token.astParent.astOperand1 == token and token.astParent.astOperand2:
                    token = token.astParent.astOperand2
                    break
                else:
                    token = token.astParent
                    if token.str == '{':
                        level = level - 1
                        levelOffsets.pop()
                        effectiveLevel = sum(levelOffsets) + level

        elif token.str == '{' :
            if not token.astOperand1:
                # Empty initializer
                self.reportError(token, 9, 2)
                return False

            token = token.astOperand1
            level = level + 1
            levelOffsets.append(0)
            isFirstElement = True
        else:
            self.reportError(token, 9, 2)
            return False

    return True

# Checks if the initializer conforms to the elements of the struct or union
# Parameters:
#   token:      root node of the initializer tree
#   elements:   the elements as specified in the declaration
def checkObjectInitializer(self, token, elements):
    if not token:
        return True

    # Initializer must start with a curly bracket
    if not token.str == '{':
        self.reportError(token, 9, 2)
        return False

    # Empty initializer is not ok { }
    if not token.astOperand1:
        self.reportError(token, 9, 2)
        return False

    token = token.astOperand1

    # Zero initializer is ok { 0 }
    if token.str == '0' :
        return True

    pos = None
    while(token):
        if token.str == ',':
            token = token.astOperand1
        else:
            if pos is None:
                pos = 0

            if token.isAssignmentOp:
                if token.astOperand1.str == '.':
                    elementName = token.astOperand1.astOperand1.str
                    pos = next((i for i, element in enumerate(elements) if element.name == elementName), len(elements))
                token = token.astOperand2

            if pos >= len(elements):
                self.reportError(token, 9, 2)
                return False

            element = elements[pos]
            if element.elementType == 'record':
                if token.isName:
                    if not token.valueType.typeScope  == element.valueType.typeScope:
                        self.reportError(token, 9, 2)
                        return False
                else:
                    subElements = getRecordElements(element.valueType)
                    if not checkObjectInitializer(self, token, subElements):
                        return False
            elif element.elementType == 'array':
                if not checkArrayInitializer(self, token, element.dimensions, element.valueType):
                    return False
            elif token.str == '{':
                self.reportError(token, 9, 2)
                return False

            # The assignment represents the astOperand
            if token.astParent.isAssignmentOp:
                token = token.astParent

            if not token == token.astParent.astOperand2:
                pos = pos + 1
                token = token.astParent.astOperand2
            else:
                token = None

    return True

