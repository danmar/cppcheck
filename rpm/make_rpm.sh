#!/bin/sh
  
rpmbuild -bb cppcheck.spec --define "_makedir $PWD/../" --define "_tmppath $PWD"
  
rm -f *_list
