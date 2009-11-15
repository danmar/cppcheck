#/bin/sh

xsltproc -o manual.html /usr/share/xml/docbook/stylesheet/nwalsh/xhtml/docbook.xsl manual.docbook

xsltproc -o intermediate-fo-file.fo /usr/share/xml/docbook/stylesheet/nwalsh/fo/docbook.xsl manual.docbook

fop -pdf manual.pdf -fo intermediate-fo-file.fo

