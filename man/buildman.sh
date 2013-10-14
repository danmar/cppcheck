#/bin/sh

xsltproc -o manual.html /usr/share/xml/docbook/stylesheet/nwalsh/xhtml/docbook.xsl manual.docbook

docbook2pdf manual.docbook
