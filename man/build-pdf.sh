#!/bin/sh
# This uses installed fonts, which vary between systems
# Segoe UI and Consolas are standard in Windows 10, DejaVu is more common on Linux
echo Building $1.pdf

MainFont="Segoe UI"
MonoFont="Consolas"

is_font_installed() {
    fontname=$1
    fc-list | grep -i "$fontname" >/dev/null
}

if ! is_font_installed "$MainFont"; then
    MainFont="DejaVu Sans"
fi
if ! is_font_installed "$MonoFont"; then
    MonoFont="DejaVu Sans Mono"
fi

# echo Using $MainFont / $MonoFont

pandoc $1.md -o $1.pdf -s --number-sections --toc \
    --pdf-engine=xelatex \
    --listings \
    -f markdown \
    -V mainfont="$MainFont" \
    -V monofont="$MonoFont" \
    -V geometry:a4paper \
    -V geometry:margin=2.4cm \
    -V subparagraph \
    -H manual-style.tex
