#!/bin/bash

WAFDIR=$(pwd)
WAF=${WAFDIR}/waf
PRINTF=$(which printf)
POSGEN=${WAFDIR}/random/position-generator

CFLAG=1
PFLAG=1
NFLAG=1
SFLAG=10
RFLAG=10
TFLAG=0
XFLAG=0
DFLAG="results"

function checkRun() {
    echo "Checking for $1"
    $WAF list 2>&1 > /dev/null | grep $1 2>&1 > /dev/null
}

source $WAFDIR/sims.conf

checkRun $NDNSIM
if [ $? -ne 0 ]; then
    echo "No scenario $NDNSIM!"
    exit 1
fi

if [ ! -x $POSGEN ]; then
    echo "No $POSGEN! Compiling..."
    cd random/
    make
    cd $WAF
fi


TYPE=( "" "--smart" "--bestr" ) 

for i in $(seq 1 $RFLAG)
do
    POS=$($POSGEN --min 0 --max 11)

    for j in "${TYPE[@]}"
    do
        RUN=$($PRINTF "$DFLAG/run-%02d" $i)
        $PRINTF "Saving in %s\n" $RUN

        if [ ! -d $RUN ]; then
            mkdir -p $RUN
        fi

         $WAF --run "$NDNSIM --results=$RUN --pos=$POS --trace $j"
         #echo "----------"
         #echo "Round $i"
         #echo "Arg $j"
         #echo "Position $POS"

     done
done
