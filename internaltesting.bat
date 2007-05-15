
@ECHO OFF

cppcheck --debug internaltesting\testdecl.cpp > internaltesting\testdecl.msg
hydfc internaltesting\testdecl.out internaltesting\testdecl.msg

cppcheck --debug internaltesting\testassign.cpp > internaltesting\testassign.msg
hydfc internaltesting\testassign.out internaltesting\testassign.msg

cppcheck --debug internaltesting\testnew.cpp > internaltesting\testnew.msg
hydfc internaltesting\testnew.out internaltesting\testnew.msg

