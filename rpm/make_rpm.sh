#!/bin/sh

cd ..

make \
	MATCHCOMPILER=yes \
	FILESDIR=/usr/share/cppcheck \
	HAVE_RULES=yes 


pkg_version="$(./cppcheck --version | cut -d ' ' -f2- | tr ' ' '_'2.2-dev)"

cd rpm

rpmbuild -bb cppcheck.spec --define "_makedir $PWD/../" \
	--define "_tmppath $PWD" \
	--define "_pkg_version $pkg_version" \
	      
rm -f *_list
