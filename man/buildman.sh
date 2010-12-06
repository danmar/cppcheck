#/bin/sh

xsltproc -o manual.html /usr/share/xml/docbook/stylesheet/nwalsh/xhtml/docbook.xsl manual.docbook

xsltproc -o intermediate-fo-file.fo /usr/share/xml/docbook/stylesheet/nwalsh/fo/docbook.xsl manual.docbook
fop -pdf manual.pdf -fo intermediate-fo-file.fo

xsltproc --stringparam generate.toc "article nop" -o intermediate-fo-file.fo /usr/share/xml/docbook/stylesheet/nwalsh/fo/docbook.xsl writing-rules.docbook
fop -pdf writing-rules.pdf -fo intermediate-fo-file.fo

