TEMPLATE = subdirs
SUBDIRS = cli test gui
CONFIG += ordered

# check target - build and run tests
check.depends = sub-test
check.commands = $$PWD/test/debug/test
QMAKE_EXTRA_TARGETS += check
