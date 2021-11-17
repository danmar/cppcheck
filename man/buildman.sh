#!/bin/sh
# To install required tools in debian:
# sudo apt-get install pandoc texlive-latex-base texlive-fonts-recommended texlive-latex-extra
# For Windows you can use the MiKTeX installer https://miktex.org/download

./build-pdf.sh manual
./build-html.sh manual

./build-pdf.sh reference-cfg-format
./build-html.sh reference-cfg-format
