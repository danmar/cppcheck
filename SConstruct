# qt4.py is required for this script to work. It was downloaded from: 
# http://clam-project.org/clam/trunk/CLAM/scons/sconstools/qt4.py
import os

# Lib
libFiles = Glob('lib/*.cpp')
libEnv = Environment()
if "gcc" in libEnv["TOOLS"]:
    libEnv.Append(CCFLAGS = ['-Wall'])
    if 'release' in COMMAND_LINE_TARGETS:
        libEnv.Append(CCFLAGS = ['-O3'])
    else:
        libEnv.Append(CCFLAGS = ['-g','-O0'])
libEnv.Append(CPPPATH = ['lib'])
libObjects = libEnv.Object(libFiles)

# Cppcheck (cli)
cliFiles = Glob('cli/*.cpp')+libObjects
cliEnv = libEnv.Clone()
cliEnv.Append(CPPPATH = ['cli'])
cppcheck = cliEnv.Program('cppcheck',cliFiles)
Default(cppcheck)

# Testrunner
testFiles = Glob('test/*.cpp')+libObjects
testEnv = libEnv.Clone()
testEnv.Append(CPPPATH = ['test'])
testrunner = testEnv.Program('testrunner', testFiles)

# GUI
QTDIR = os.popen('qmake -query QT_INSTALL_DATA').read() 
pkgpath = os.environ.get("PKG_CONFIG_PATH", "")
pkgpath += ":%s/lib/pkgconfig" % QTDIR
os.environ["PKG_CONFIG_PATH"] = pkgpath
os.environ["QTDIR"] = QTDIR
guiEnv = libEnv.Clone(tools=["default", "qt4"], toolpath=['.'])
guiEnv.Append(CPPPATH = ['gui'])
guiEnv.EnableQt4Modules(["QtGui", "QtCore"])
rccs = [guiEnv.Qrc("gui/gui.qrc", QT4_QRCFLAGS="-name CppcheckGUI")]
uiFiles = Glob('gui/*.ui')
uis = [guiEnv.Uic4(ui) for ui in uiFiles]
guiFiles = Glob('gui/*.cpp')+libObjects
gui = guiEnv.Program(target="gui_cppcheck", source=[rccs, guiFiles])

# Execute testrunner
AlwaysBuild(Alias("test", testrunner, "./$SOURCE"))

# All
Alias('all',[testrunner,cppcheck,gui])

# Release
Alias('release',[cppcheck])
