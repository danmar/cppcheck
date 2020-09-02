Name:           cppcheck
Version:    	%{_pkg_version}
Release:    	0
Summary:        A tool for static C/C++ code analysis
License:        GPL-3.0-or-later      
Group:          Development/Languages/C and C++
Url:            https://github.com/danmar/%{name}
  
%define templist %{_tmppath}/tmp_list
%define finallist %{_tmppath}/entire_list
  
%description
Cppcheck is a static analysis tool for C/C++ code. Unlike C/C++
compilers and many other analysis tools it does not detect syntax
errors in the code. Cppcheck primarily detects the types of bugs that
the compilers normally do not detect. The goal is to detect only real
errors in the code (i.e. have zero false positives).
   
%install
pushd %{_makedir}
  
make DESTDIR=$RPM_BUILD_ROOT FILESDIR=/usr/share/cppcheck install
  
popd
  
pushd $RPM_BUILD_ROOT
  
find . -type d -name cppcheck > %{templist}
find . -type f >> %{templist}
  
# remove the starting dot
cat %{templist} | cut -d . -f 2- > %{finallist}
  
%files -f %{finallist}
 
%changelog
* Mon Aug 31 2020 Bianca-Sidonia Hidos <https://github.com/HidosBiancaSidonia> %{_pkg_version}
- Initial RPM release
