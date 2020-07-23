#!/bin/sh
# To install required tools in debian:
# sudo apt-get install pandoc texlive-latex-base texlive-fonts-recommended texlive-latex-extra

pandoc manual.md -o manual.pdf -s --number-sections --toc -f markdown --pdf-engine=xelatex --listings -V mainfont="Segoe UI" -V monofont="Consolas" -V geometry:a4paper -V geometry:margin=2.4cm -H manual-style.tex

pandoc manual.md -o manual.html -s --number-sections --toc --css manual.css

pandoc reference-cfg-format.md -o reference-cfg-format.pdf -s --number-sections --toc
pandoc reference-cfg-format.md -o reference-cfg-format.html -s --number-sections --toc
