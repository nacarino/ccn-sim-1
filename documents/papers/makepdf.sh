#!/bin/bash

OBJ=$1
OUT=$2

if [ -z "$OUT" ]; then
    echo "No output name given!"
    exit 1
fi

if [ -z "$OBJ" ]; then
    echo "No tex file given!"
    exit 1
else
    platex $OBJ.tex
    bibtex $OBJ
    platex $OBJ
    platex $OBJ
    dvips -ta4 $OBJ.dvi
    ps2pdf $OBJ.ps
    pdftk $OBJ.pdf update_info data.txt output $OUT.pdf
fi
