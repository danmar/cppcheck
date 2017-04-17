cppcheck-htmlreport

This is a little utility to generate a html report of a XML file produced by
cppcheck.

The utility is implemented in Python (2.7+) and requires the pygments module
to generate syntax highlighted source code.
If you are using a Debian based Linux system, the pygments package can be 
installed by following command:
$ sudo apt-get install python-pygments

For more information run './cppcheck-htmlreport --help'


--- Old python compatibility ---
The code requires python 2.7+ for collections Counter import.

Quick-fix tested in Python 2.6.6 on CentOS6 is to add the following at the begininning of ./cppcheck-htmlreport:
1)
from future.standard_library import install_aliases
install_aliases()

2) change the line 258:
html_unescape_table = {
    "&quot;" : '"', 
    "&apos;" : "'"
}
#{v: k for k, v in html_escape_table.items()}

