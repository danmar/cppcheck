# Holds information about an array, struct or union's element definition.
class ElementDef:
    def __init__(self, elementType, name, valueType, dimensions = None):
        self.elementType = elementType      # 'array', 'record' or 'value'
        self.name = str(name)
        self.valueType = valueType
        self.children = []
        self.dimensions = dimensions
        self.parent = None

        self.isDesignated = False
        self.isPositional = False
        self.numInits = 0
        self.childIndex = -1

        self.isFlexible = False
        self.structureViolationToken = None

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

    @property
    def isArray(self):
        return self.elementType == 'array'

    @property
    def isRecord(self):
        return self.elementType == 'record'

    @property
    def isValue(self):
        return self.elementType == 'value'


    def getLongName(self):
        return self.parent.getLongName() + "." + self.name if self.parent else self.name

    def getInitDump(self):
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
                childDumps.append(c.getInitDump())
            myDump += "{ " + ", ".join(childDumps) + " }"

        return myDump

    def addChild(self, child):
        self.children.append(child)
        child.parent = self

    def getNextChild(self):
        self.childIndex += 1
        return self.getChildByIndex(self.childIndex)

    def getChildByIndex(self, index):
        if self.isFlexible:
            while len(self.children) <= index:
                createChild(self, self.flexibleToken, len(self.children))
        return self.children[index] if index >= 0 and len(self.children) > index else None

    def getChildByName(self, name):
        for c in self.children:
            if c.name == name:
                return c
        return None

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
        next_child = current.getChildByIndex(0)
        while next_child:
            current = next_child
            next_child = current.getChildByIndex(0)
        return current

    def getLastValueElement(self):
        current = self
        # Move to last child as long as children exists
        while len(current.children) > 0:
            current = current.children[-1]
        return current

    def getChildByValueElement(self, ed):
        potentialChild = ed
        while potentialChild and not potentialChild in self.children:
            potentialChild = potentialChild.parent

        return self.children[self.children.index(potentialChild)] if potentialChild else None

    def getEffectiveLevel(self):
        if self.parent and self.parent.elementType == "array":
            return self.parent.getEffectiveLevel() + 1
        else:
            return 0

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

    def markStuctureViolation(self, token):
        if self.name == '->':
            self.children[0].markStuctureViolation(token)
        elif not self.structureViolationToken:
            self.structureViolationToken = token

    def markAsFlexibleArray(self, token):
        self.flexibleToken = token;
        self.isFlexible = True

    def markAsCurrent(self):
        if self.parent:
            if self.name == '<-':
                self.parent.childIndex = self.parent.children.index(self.children[0])
            else:
                self.parent.childIndex = self.parent.children.index(self)

            self.parent.markAsCurrent()

    def isAllChildrenSet(self):
        myself = len(self.children) == 0 and (self.isDesignated or self.isPositional)
        mychildren =  len(self.children) > 0 and all([child.isAllChildrenSet() for child in self.children])
        return myself or mychildren

    def isAllSet(self):
        return all([child.isPositional or child.isDesignated for child in self.children])

    def isOnlyDesignated(self):
        return all([not child.isPositional for child in self.children])

    def isMisra92Compliant(self):
        return self.structureViolationToken is None and all([child.isMisra92Compliant() for child in self.children])

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

    def isMisra95Compliant(self):
        return not self.isFlexible or all([not child.isDesignated for child in self.children])


def misra_9_x(self, data, rule, rawTokens = None):
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

        if variable.isArray or variable.isClass:
            ed = getElementDef(nameToken, rawTokens)
            parseInitializer(ed, eq.astOperand2, rule)
            # print(rule, nameToken.str + '=', ed.getInitDump())
            if rule == 902 and not ed.isMisra92Compliant():
                self.reportError(nameToken, 9, 2)
            if rule == 903 and not ed.isMisra93Compliant():
                self.reportError(nameToken, 9, 3)
            if rule == 904 and not ed.isMisra94Compliant():
                self.reportError(nameToken, 9, 4)
            if rule == 905 and not ed.isMisra95Compliant():
                self.reportError(nameToken, 9, 5)

def getElementDef(nameToken, rawTokens = None):
    if nameToken.variable.isArray:
        ed = ElementDef("array", nameToken.str, nameToken.valueType)
        createArrayChildrenDefs(ed, nameToken.astParent, rawTokens)
    elif nameToken.variable.isClass:
        ed = ElementDef("record", nameToken.str, nameToken.valueType)
        createRecordChildrenDefs(ed)
    else:
        ed = ElementDef("value", nameToken.str, nameToken.valueType)
    return ed

def createArrayChildrenDefs(ed, token, rawTokens = None):
    if token.str == '[':
        if rawTokens is not None:
            foundToken = next(rawToken for rawToken in rawTokens if rawToken.file == token.file and rawToken.linenr == token.linenr and rawToken.column == token.column)

            if foundToken and foundToken.next and foundToken.next.str == ']':
                ed.markAsFlexibleArray(token)

        if token.astOperand2 is not None:
            for i in range(token.astOperand2.getKnownIntValue()):
                createChild(ed, token, i)
        else:
            ed.markAsFlexibleArray(token)


def createChild(ed, token, name):
    if token.astParent and token.astParent.str == '[':
        child = ElementDef("array", name, ed.valueType)
        createArrayChildrenDefs(child, token.astParent)
    else:
        if ed.valueType.type == "record":
            child = ElementDef("record", name, ed.valueType)
            createRecordChildrenDefs(child)
        else:
            child = ElementDef("value", name, ed.valueType)

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

            if not ed.isArray:
                ed.markStuctureViolation(token)

            ed = ed.getChildByIndex(chIndex)

        elif token.str == '.':
            name = ""
            if token.astOperand2 is not None:
                name = token.astOperand2.str
            elif token.astOperand1 is not None:
                name = token.astOperand1.str

            if not ed.isRecord:
                ed.markStuctureViolation(token)

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

    dummyRoot = ElementDef('array', '->', root.valueType)
    dummyRoot.children = [root]

    rootStack = []
    root = dummyRoot
    ed = root.getFirstValueElement()
    isFirstElement = False
    isDesignated = False

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
                # Make sure ed points to valueElement
                ed = ed.getFirstValueElement()

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

                    root.markStuctureViolation(token)

                    # Fake dummy as nextChild (of current root)
                    nextChild = dummyRoot

            if token.astOperand1:
                root = nextChild
                token = token.astOperand1
                isFirstElement = True
            else:
                # {}
                if root.name == '<-':
                    root.parent.markStuctureViolation(token)
                else:
                    root.markStuctureViolation(token)
                ed = None
                unwindAndContinue()

        else:
            if ed and ed.isValue:
                if isFirstElement and token.str == '0' and token.next.str == '}':
                    # Zero initializer causes recursive initialization
                    root.initializeChildren()
                elif  token.isString and ed.valueType.pointer > 0:
                    if ed.valueType.pointer - ed.getEffectiveLevel() == 1:
                        if ed.parent != root:
                            root.markStuctureViolation(token)
                        ed.setInitialized(isDesignated)
                    elif ed.valueType.pointer == ed.getEffectiveLevel():
                        if(root.name != '->' and ed.parent.parent != root) or (root.name == '->' and root.children[0] != ed.parent):
                            root.markStuctureViolation(token)
                        else:
                            ed.parent.setInitialized(isDesignated)
                        ed.parent.initializeChildren()
                else:
                    if ed.parent != root:
                        # Check if token is correct value type for root.children[?]
                        child = root.getChildByValueElement(ed)
                        if child.elementType != 'record' or token.valueType.type != 'record' or child.valueType.typeScope != token.valueType.typeScope:
                            root.markStuctureViolation(token)
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
