#!/bin/bash
g++ -O2 -o democlient.cgi -Icppcheck-1.55/lib democlient.cpp cppcheck-1.55/lib/*.cpp
mv democlient.cgi /home/project-web/cppcheck/cgi-bin/
chmod +rx /home/project-web/cppcheck/cgi-bin/democlient.cgi
