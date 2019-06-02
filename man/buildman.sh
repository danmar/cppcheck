#!/bin/sh

pandoc manual.md  -o manual.pdf -s --pdf-engine=pdflatex --number-sections
pandoc manual.md  -o manual.html -s --number-sections

