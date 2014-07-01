#!/bin/bash

OBJ=$1

if [ -z "$OBJ" ]; then
    echo "No tex file given!"
    exit 1
else
    platex $OBJ.tex
    bibtex $OBJ
    platex $OBJ
    platex $OBJ
    dvipdf $OBJ.dvi $OBJ.pdf
fi
