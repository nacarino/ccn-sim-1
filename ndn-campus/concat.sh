#!/bin/bash

head -n1 "$1" > "$2"

if [ -z "$4" ];then
    tail -q -n+2 $(echo $3) >> "$2"
else
    tail -q -n1 $(echo $3) >> "$2"
fi
