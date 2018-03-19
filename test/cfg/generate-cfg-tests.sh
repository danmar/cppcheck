#!/bin/bash
cd ~/cppcheck
make generate_cfg_tests
./generate_cfg_tests cfg/avr.cfg > test/cfg/generated-cfg-tests-avr.cpp
./generate_cfg_tests cfg/bsd.cfg > test/cfg/generated-cfg-tests-bsd.cpp
./generate_cfg_tests cfg/gnu.cfg > test/cfg/generated-cfg-tests-gnu.cpp
./generate_cfg_tests cfg/motif.cfg > test/cfg/generated-cfg-tests-motif.cpp
./generate_cfg_tests cfg/posix.cfg > test/cfg/generated-cfg-tests-posix.cpp
./generate_cfg_tests cfg/qt.cfg > test/cfg/generated-cfg-tests-qt.cpp
./generate_cfg_tests cfg/sdl.cfg > test/cfg/generated-cfg-tests-sdl.cpp
./generate_cfg_tests cfg/sfml.cfg > test/cfg/generated-cfg-tests-sfml.cpp
./generate_cfg_tests cfg/std.cfg > test/cfg/generated-cfg-tests-std.cpp
./generate_cfg_tests cfg/windows.cfg > test/cfg/generated-cfg-tests-windows.cpp
./generate_cfg_tests cfg/wxwidgets.cfg > test/cfg/generated-cfg-tests-wxwidgets.cpp
