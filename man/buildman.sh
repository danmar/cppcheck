#!/bin/sh
# To install required tools in debian:
# sudo apt-get install pandoc texlive-latex-base texlive-fonts-recommended texlive-latex-extra

pandoc manual.md  -o manual.pdf -s --number-sections
pandoc manual.md  -o manual.html -s --number-sections

