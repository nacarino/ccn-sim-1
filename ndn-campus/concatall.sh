#!/bin/bash

head -n1 "$1" > "$2"
tail -q -n+2 $(echo $3) >> "$2"
