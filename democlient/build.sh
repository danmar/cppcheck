#!/bin/bash

# this script downloads and builds the democlient
# syntax:
# ./build 1.56

# cppcheck lib folder
cppchecklib=cppcheck-$1/lib

echo Download..
wget https://downloads.sourceforge.net/project/cppcheck/cppcheck/$1/cppcheck-$1.tar.bz2

echo Unpack..
tar xjvf cppcheck-$1.tar.bz2
rm cppcheck-$1.tar.bz2
rm cppcheck-$1/Changelog

echo Building..
g++ -O2 -o democlient-$1.cgi -I$cppchecklib cppcheck-$1/democlient/democlient.cpp $cppchecklib/*.cpp

echo Copy cgi to webspace..
cp democlient-$1.cgi /home/project-web/cppcheck/cgi-bin/democlient.cgi
chmod +rx /home/project-web/cppcheck/cgi-bin/democlient.cgi

echo Done!

