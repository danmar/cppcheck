#!/usr/bin/env python
#
# cppcheck addon for ROS naming conventions
#
# Example usage (variable name must start with lowercase, function name must start with uppercase):
# $ cppcheck --dump path-to-src/
# $ python addons/ros_naming.py path-to-src/*.dump
#
#  THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, 
#  BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
#  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
#  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import sys
import cppcheckdata
import re

#Check trailing underscore
RE_TRAILING_UNDERSCORE = '.*\_$'
#Check underscore
RE_UNDERSCORE = '.*\_'
#Check capital letters
RE_CAPITAL = '.*[A-Z]'
#Check start small letter
RE_SMALL_START = '.*^[a-z]'
#Check start small letter
RE_CAPITAL_START = '.*^[A-Z]'
#Check start g_
RE_G_START = '.*^([g]\_)'

FoundError = False

def reportError(token, severity, msg):
    global FoundError
    FoundError = True
    sys.stderr.write(
        '[' + token.file + ':' + str(token.linenr) + '] (' + severity + ') ros_naming.py: ' + msg + '\n')

def parseFileNaming(data):
    global FoundError
    FoundError = False
    print('Parsing File naming ...')              
    #Check capital
    res = re.match(RE_CAPITAL, data)
    if res:
        FoundError = True
        sys.stderr.write('[ File' + arg + '] (style) ros_naming.py: ' + arg + ' violates naming convention : under_scored \n')
        
    if not FoundError:
        print('File ' + arg[:-5] + ' naming is correct')
    
def parseNamespaceNaming(arg, data): 
    global FoundError
    FoundError = False
    print('Parsing Namespace naming ...')
    for tk in data.rawTokens:
        if (tk.str == 'namespace'):              
                #Check capital
                res = re.match(RE_CAPITAL, tk.next.str)
                if res:
                    reportError(tk.next, 'style', 'Namespace ' +
                                tk.next.str + ' violates naming convention : under_scored')
                #Check no trailing underscore
                res = re.match(RE_TRAILING_UNDERSCORE, tk.next.str)
                if res:
                    reportError(tk.next, 'style', 'Namespace ' +
                                tk.next.str + ' violates naming convention : Remove trailing _')
    
    if not FoundError:
        print('Namespace naming in ' + arg[:-5] + ' is correct')
                
def parseVarNaming(arg, data):   
    global FoundError
    FoundError = False
    for cfg in data.configurations:            
        print('Parsing Variable naming ...')
        for var in cfg.variables:
            # Check if Public or Private
            if (var.access == 'Public' or var.access == 'Private'):
                #Get scope
                scope = var.scope
                
                #Check if belongs to Struct or Class
                '''
                if (scope.type == 'Struct'):
                    print(var.nameToken.str + ' belongs to Struct: ' + scope.className + ' and is ' + var.access)
                elif (scope.type == 'Class'):
                    print(var.nameToken.str + ' belongs to Class: ' + scope.className + ' and is ' + var.access)
                '''
                #Check trailing underscore 
                res = re.match(RE_TRAILING_UNDERSCORE, var.nameToken.str)
                if not res:
                    reportError(var.typeStartToken, 'style', 'Variable ' +
                                var.nameToken.str + ' violates naming convention : Add trailing _')
                #Check no capital letters
                res = re.match(RE_CAPITAL, var.nameToken.str)
                if res:
                    reportError(var.typeStartToken, 'style', 'Variable ' +
                                var.nameToken.str + ' violates naming convention : under_score_')
                    
            elif (var.access == 'Global'):
                #Check g_ start
                res = re.match(RE_G_START, var.nameToken.str)
                if not res:
                    reportError(var.typeStartToken, 'style', 'Variable ' +
                                var.nameToken.str + ' violates naming convention : g_ at start')
                #Check no capital letters
                res = re.match(RE_CAPITAL, var.nameToken.str)
                if res:
                    reportError(var.typeStartToken, 'style', 'Variable ' +
                                var.nameToken.str + ' violates naming convention : g_under_score')  
                #Check no trailing underscore
                res = re.match(RE_TRAILING_UNDERSCORE, var.nameToken.str)
                if res:
                    reportError(var.typeStartToken, 'style', 'Variable ' +
                                var.nameToken.str + ' violates naming convention : Remove trailing _')
                    
            else:
                if (var.nameToken != None):
                    #Check no g_ start
                    res = re.match(RE_G_START, var.nameToken.str)
                    if res:
                        reportError(var.typeStartToken, 'style', 'Variable ' +
                                    var.nameToken.str + ' violates naming convention : Remove g_ at start')
                    #Check no capital letters
                    res = re.match(RE_CAPITAL, var.nameToken.str)
                    if res:
                        reportError(var.typeStartToken, 'style', 'Variable ' +
                                    var.nameToken.str + ' violates naming convention : under_score')  
                    #Check no trailing underscore
                    res = re.match(RE_TRAILING_UNDERSCORE, var.nameToken.str)
                    if res:
                        reportError(var.typeStartToken, 'style', 'Variable ' +
                                    var.nameToken.str + ' violates naming convention : Remove trailing _')
    if not FoundError:
        print('Variable naming in ' + arg[:-5] + ' is correct')
                
def parseFncNaming(arg, data):   
    global FoundError
    FoundError = False
    for cfg in data.configurations:
        print('Parsing Function naming ...')
        for fnc in cfg.functions:
            #Check if it is constructor
            if (fnc.type == 'Constructor' or fnc.type == 'Destructor'):
                #print(fnc.type + ' : ' + fnc.name)
                #Check capital start
                res = re.match(RE_CAPITAL_START, fnc.name)
                if not res:
                    reportError(fnc.tokenDef, 'style', 'Class ' +
                                fnc.name + ' ' + fnc.type + ' violates naming convention : CamelCase')
                #Check underscore
                res = re.match(RE_UNDERSCORE, fnc.name)
                if res:
                    reportError(fnc.tokenDef, 'style', 'Class ' +
                                fnc.name + ' violates naming convention : CamelCase')
                    
            elif (fnc.type == 'Function'):
                #print(fnc.type + ' : ' + fnc.name)
                
                #Check underscore
                res = re.match(RE_UNDERSCORE, fnc.name)
                if res:
                    reportError(fnc.tokenDef, 'style', 'Function ' +
                                fnc.name + ' violates naming convention : camelCase')
                    
                #Check start
                res = re.match(RE_SMALL_START, fnc.name)
                if not res:
                    reportError(fnc.tokenDef, 'style', 'Function ' +
                                fnc.name + ' violates naming convention : camelCase')
            else:
                print('Other')
            
    if not FoundError:
        print('Function naming in ' + arg[:-5] + ' is correct')

            
for arg in sys.argv[1:]:
    data = cppcheckdata.parsedump(arg)
    parseFileNaming(arg)
    parseNamespaceNaming(arg, data)
    parseVarNaming(arg, data)
    parseFncNaming(arg, data)
    
    
