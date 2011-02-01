#/bin/sh

xsltproc -o manual.html /usr/share/xml/docbook/stylesheet/nwalsh/xhtml/docbook.xsl manual.docbook

xsltproc -o intermediate-fo-file.fo /usr/share/xml/docbook/stylesheet/nwalsh/fo/docbook.xsl manual.docbook
fop -pdf manual.pdf -fo intermediate-fo-file.fo

xsltproc --stringparam generate.toc "article nop" -o intermediate-fo-file.fo /usr/share/xml/docbook/stylesheet/nwalsh/fo/docbook.xsl writing-rules-1.docbook
fop -pdf writing-rules-1.pdf -fo intermediate-fo-file.fo

xsltproc --stringparam generate.toc "article nop" -o intermediate-fo-file.fo /usr/share/xml/docbook/stylesheet/nwalsh/fo/docbook.xsl writing-rules-2.docbook
fop -pdf writing-rules-2.pdf -fo intermediate-fo-file.fo

xsltproc --stringparam generate.toc "article nop" -o intermediate-fo-file.fo /usr/share/xml/docbook/stylesheet/nwalsh/fo/docbook.xsl writing-rules-3.docbook
fop -pdf writing-rules-3.pdf -fo intermediate-fo-file.fo

xsltproc --stringparam generate.toc "article nop" -o intermediate-fo-file.fo /usr/share/xml/docbook/stylesheet/nwalsh/fo/docbook.xsl cppcheck-design.docbook
fop -pdf cppcheck-design.pdf -fo intermediate-fo-file.fo

