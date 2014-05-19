#!/bin/bash

WAFDIR=$(pwd)
WAF=${WAFDIR}/waf
PRINTF=$(which printf)

CFLAG=1
PFLAG=1
NFLAG=1
SFLAG=10
RFLAG=1
TFLAG=""
XFLAG=""
DFLAG="results"

function usage() {
echo "Tiny script to automize running of a ns3 scenario"
echo "Options:"
echo "    -c NUM    Max number of clients (consumers) for the scenario. Default [$CFLAG]. Starts at 1"
echo "    -n NUM    Max number of networks the scenario will have. Default [$NFLAG]. Starts at 1"
echo "    -p NUM    Max number of servers (producers) for the scenario. Default [$PFLAG]. Starts at 1"
echo "    -r NUM    Number of times to run the scenario. Default [$RFLAG]"
echo "    -s NUM    Size, in MB of the content to be distributed. Default [$SFLAG]"
echo "    -t        Run TCP scenario. Default [$TFLAG]"
echo "    -x        Run NDN scenario. Default [$XFLAG]"
echo "    -d DIR    Directory to place the results. Default [$DFLAG]"
echo ""
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
        TFLAG="-t"
        ;;
    x)
        XFLAG="-x"
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

for i in $(seq 1 $NFLAG)
do
    for j in $(seq 1 $PFLAG)
    do
        for k in $(seq 1 $CFLAG)
        do
            $WAFDIR/run-sim.sh -n $i -p $j -c $k -r $RFLAG -d $DFLAG -r $RFLAG -s $SFLAG $TFLAG $XFLAG
        done
    done
done
