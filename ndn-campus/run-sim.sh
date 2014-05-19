#!/bin/bash

WAFDIR=$(pwd)
WAF=${WAFDIR}/waf
CONTSIZE=${WAFDIR}/random/content-size-generator
PRINTF=$(which printf)

CFLAG=1
PFLAG=1
NFLAG=1
SFLAG=10
RFLAG=1
TFLAG=0
XFLAG=0
DFLAG="results"

function usage() {
    echo "Tiny script to automize running of a ns3 scenario"
    echo "Options:"
    echo "    -c NUM    Number of clients (consumers) for the scenario. Default [$CFLAG]"
    echo "    -n NUM    Number of networks the scenario will have. Default [$NFLAG]"
    echo "    -p NUM    Number of servers (producers) for the scenario. Default [$PFLAG]"
    echo "    -r NUM    Number of times to run the scenario. Default [$RFLAG]"
    echo "    -s NUM    Size, in MB of the content to be distributed. Default [$SFLAG]"
    echo "    -t        Run TCP scenario. Default [$TFLAG]"
    echo "    -x        Run NDN scenario. Default [$XFLAG]"
    echo "    -d DIR    Directory to place the results. Default [$DFLAG]"
    echo ""
}

function checkRun() {
    echo "Checking for $1"
    $WAF list 2>&1 > /dev/null | grep $1 2>&1 > /dev/null
}

while getopts "r:d:c:s:n:p:htx" OPT
do
    case $OPT in
    c)
        CFLAG=$OPTARG  
        ;;
    p)
        PFLAG=$OPTARG
        ;;
    n)
        NFLAG=$OPTARG
        ;;
    s)
        SFLAG=$OPTARG
        ;;
    t)
        TFLAG=1
        ;;
    x)
        XFLAG=1
        ;;
    d)
        DFLAG=$OPTARG
        ;;
    r)
        RFLAG=$OPTARG
        ;;
    \?)
        echo "Invalid option: -$OPTARG" >&2
        exit 1
        ;;
    :)
        echo "Option -$OPTARG requires an argument." >&2
        exit 1
        ;;
    h)
        usage
        exit 0
        ;;
    *)
        usage
        exit 1
        ;;
    esac
done

if [ ! -x $CONTSIZE ]; then
    echo "No $CONTSIZE! Compiling..."
    cd random/
    make
    cd $WAF
fi

if [ ! -e $WAFDIR/sims.conf ]; then
    echo "$WAFDIR/sims.conf does not exist! Must be created to run the simulations!"
    exit 1
fi

source $WAFDIR/sims.conf

for i in $(seq 1 $RFLAG)
do

    if [ $TFLAG -eq 0 ] && [ $XFLAG -eq 0 ]; then
        echo "No -t or -x flag set! No simulations to run!"
        exit 0
    fi

    BYTES=$($CONTSIZE --avg $SFLAG)

    $PRINTF "Running %d clients, %d servers, %d networks, %d contentsize, Round %d\n" $CFLAG $PFLAG $NFLAG $BYTES $RFLAG

    RUN=$($PRINTF "$DFLAG/run-%02d" $i)

    if [ ! -d $RUN ]; then
        mkdir -p $RUN
    fi

    $PRINTF "Saving in %s\n" $RUN

    if [ $TFLAG -eq 1 ]; then
        if [ -z $TCPSIM ]; then
            echo "$WAFDIR/sims.conf does not declare \$TCPSIM!"
            exit 1
        fi

        checkRun $TCPSIM
        if [ $? -ne 0 ]; then
            echo "No scenario $TCPSIM!"
            exit 1
        fi

        $WAF --run "$TCPSIM --clients=$CFLAG --contentsize=$BYTES --networks=$NFLAG --servers=$PFLAG  --results=$RUN"
    fi

    if [ $XFLAG -eq 1 ]; then
        if [ -z $NDNSIM ]; then
            echo "$WAFDIR/sims.conf does not declare \$NDNSIM!"
            exit 1
        fi

        checkRun $NDNSIM
        if [ $? -ne 0 ]; then
            echo "No scenario $NDNSIM!"
            exit 1
        fi

        $WAF --run "$NDNSIM --clients=$CFLAG --contentsize=$BYTES --networks=$NFLAG --servers=$PFLAG  --results=$RUN"
    fi
done
