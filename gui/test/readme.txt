GUI tests
===========================

As the GUI uses Qt framework, the GUI tests also use Qt's Testlib. This is
totally different test framework than lib/cli is using. By principle each
testcase is compiled as an own runnable binary.

Compiling
---------

To compile all the tests run in root directory of tests:
 - qmake ; make

You can also (re)compile single test by CD:ing to the directory where test
(source) resides and running:
 - qmake ; make

Running
-------

As each test is compiled as single executable binary you can run the test just
by running the executable.

You can get from
 http://bitbucket.org/kimmov/testrun
a script which runs all the tests and collects the results.

