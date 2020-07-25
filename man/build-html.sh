#!/bin/sh
echo Building $1.html
pandoc $1.md -o $1.html -s --number-sections --toc --css manual.css
