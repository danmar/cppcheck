#!/bin/bash
g++ -O2 -o democlient.cgi -Icppcheck-1.56/lib democlient.cpp cppcheck-1.56/lib/*.cpp
mv democlient.cgi /home/project-web/cppcheck/cgi-bin/
chmod +rx /home/project-web/cppcheck/cgi-bin/democlient.cgi
