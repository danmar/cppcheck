# qt4.py is required for this script to work. It was downloaded from: 
# http://clam-project.org/clam/trunk/CLAM/scons/sconstools/qt4.py
import os

# Lib
libFiles = Glob('lib/*.cpp')
libEnv = Environment()
if "gcc" in libEnv["TOOLS"]:
	libEnv.Append(CCFLAGS = ['-Wall', '-Wextra'])
	if 'release' in COMMAND_LINE_TARGETS:
		libEnv.Append(CCFLAGS = ['-O3'])
	elif 'coverage' in COMMAND_LINE_TARGETS:
		libEnv.Append(CCFLAGS = ['-g', '-fprofile-arcs', '-ftest-coverage'])
		libEnv.Append(LINKFLAGS = ['-g', '-fprofile-arcs', '-ftest-coverage'])
	elif 'ccwin32' in COMMAND_LINE_TARGETS:
		libEnv.Replace(CXX = 'i586-mingw32msvc-g++')
		libEnv.Replace(CCFLAGS = ['-O3'])
		libEnv.Replace(LIBS = ['shlwapi'])
	else:
		libEnv.Append(CCFLAGS = ['-g','-O0'])

libEnv.Append(CPPPATH = ['lib'])
libObjects = libEnv.Object(libFiles)

# Cppcheck (cli)
cliFiles = Glob('cli/*.cpp')+libObjects
cliEnv = libEnv.Clone()
cliEnv.Append(CPPPATH = ['cli'])
if "gcc" in libEnv["TOOLS"]:
	cliEnv.Append(CCFLAGS = ['-pedantic'])
cppcheck = cliEnv.Program('cppcheck',cliFiles)
Alias('cppcheck',[cppcheck])
Default(cppcheck)

# Testrunner
testFiles = Glob('test/*.cpp')+libObjects
testEnv = libEnv.Clone()
testEnv.Append(CPPPATH = ['test'])
if "gcc" in libEnv["TOOLS"]:
	testEnv.Append(CCFLAGS = ['-pedantic'])
testrunner = testEnv.Program('testrunner', testFiles)
Alias('testrunner',[testrunner])

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
Alias('gui',[gui])

# Execute testrunner
test = AlwaysBuild(Alias("test", testrunner, "./$SOURCE"))

# All
Alias('all',[testrunner,cppcheck,gui])

# Release
Alias('release',[cppcheck])

# ccwin32 (Cross compile win32 in Linux)
ccwin32 = Alias('ccwin32',[cppcheck],Move('cppcheck.exe','cppcheck'))
Clean(ccwin32,'cppcheck.exe')

# Coverage
Alias('coverage',[test],[
	'rm -r coverage_report',
	'gcov lib/*.cpp -o lib/',
	'lcov --directory ./ --capture --output-file lcov.info -b ./',
	'genhtml lcov.info -o coverage_report',
	'rm lib/*.gcda',
	'rm lib/*.gcno',
	'rm test/*.gcda',
	'rm test/*.gcno',
	'rm lcov.info'])

Help("""
       Type: 'scons'             - cppcheck (cli)
             'scons cppcheck'    - cppcheck (cli)
             'scons gui'         - gui_cppcheck
             'scons testrunner'  - testrunner
             'scons test'        - testrunner + execute tests
             'scons all'         - gui+cppcheck+testrunner
             'scons release'     - cppcheck release version
             'scons release gui' - cppcheck+gui release versions
             'scons coverage'    - coverage report
             'scons ccwin32'     - Cross compile win32 (cli) in Linux
             'scons -c all'      - clean all
       """)

