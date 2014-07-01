#!/bin/bash

FILE=CCNWireless-rate-trace
VAR=( "flood" "bestr" "smart" )
DEF=01-001-012.txt

cd results

for j in "${VAR[@]}"
do
    TMP=$FILE-$j
    ../concat.sh run-01/$TMP-$DEF $TMP-matome.txt "run-*/$TMP*"
done

