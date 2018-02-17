# Rough metrics
#
# complexity is calculated according to the "shortcut" here:
# https://stackoverflow.com/questions/9097987/calculation-of-cyclomatic-complexity

import cppcheckdata
import sys

class FunctionMetrics:
    lines = 0
    complexity = 0

def getMetrics(data):
    variables = {}
    functions = {}
    for cfg in data.configurations:
        for func in cfg.functions:
            if func.tokenDef is None:
                continue
            location = func.tokenDef.file + ':' + str(func.tokenDef.linenr)
            print('name:'+func.name)
            print('  location:' + location)
            
            indent = 0
            lines = 0
            complexity = 0
            tok = func.tokenDef
            while tok:
                #print(tok.str)
                if tok.str == '{':
                    indent = indent + 1
                    if indent > 1:
                        lines = lines + 1
                    complexity = complexity + 1
                elif tok.str == '}':
                    indent = indent - 1
                    if indent < 1:
                        break
                elif tok.str == ';':
                    if indent <= 0:
                        break
                    lines = lines + 1
                elif tok.str == '(':
                    tokens = [tok.astOperand2]
                    while len(tokens) > 0:
                        tok2 = tokens.pop()
                        if tok2 is None:
                            continue
                        if tok2.str != '&&' and tok2.str != '||':
                            continue
                        complexity = complexity + 1
                        tokens.append(tok2.astOperand1)
                        tokens.append(tok2.astOperand2)
                    tok = tok.link
                tok = tok.next

            print('  lines:' + str(lines))
            print('  complexity:' + str(complexity))
    return

for arg in sys.argv[1:]:
    data = cppcheckdata.parsedump(arg)
    getMetrics(data)
