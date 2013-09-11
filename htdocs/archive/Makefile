all:
	gcc -std=c99 -o addfile.cgi addfile.c validatexml.c
	gcc -std=c99 -Wno-attributes -o createzip.cgi createzip.c validatexml.c
	gcc -std=c99 -o deletefile.cgi deletefile.c validatexml.c
	gcc -std=c99 -o edit.cgi edit.c validatexml.c
	gcc -std=c99 -o renamefile.cgi renamefile.c validatexml.c
	gcc -std=c99 -o report.cgi report.c validatexml.c
	gcc -std=c99 -o setfiledata.cgi setfiledata.c validatexml.c

movecgi:	all
	mv addfile.cgi /home/project-web/cppcheck/cgi-bin/
	chmod +rx /home/project-web/cppcheck/cgi-bin/addfile.cgi
	mv createzip.cgi /home/project-web/cppcheck/cgi-bin/archive.zip
	chmod +rx /home/project-web/cppcheck/cgi-bin/archive.zip
	mv deletefile.cgi /home/project-web/cppcheck/cgi-bin/
	chmod +rx /home/project-web/cppcheck/cgi-bin/deletefile.cgi
	mv edit.cgi /home/project-web/cppcheck/cgi-bin/
	chmod +rx /home/project-web/cppcheck/cgi-bin/edit.cgi
	mv renamefile.cgi /home/project-web/cppcheck/cgi-bin/
	chmod +rx /home/project-web/cppcheck/cgi-bin/renamefile.cgi
	mv report.cgi /home/project-web/cppcheck/cgi-bin/
	chmod +rx /home/project-web/cppcheck/cgi-bin/report.cgi
	mv setfiledata.cgi /home/project-web/cppcheck/cgi-bin/
	chmod +rx /home/project-web/cppcheck/cgi-bin/setfiledata.cgi
