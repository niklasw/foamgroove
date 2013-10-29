#!/bin/bash
#
# Parse solver log file for PID and kill it.
# Optionally send write signal instead.

prog=$(basename $0)

function help()
{
    cat << EOF
Usage: $prog [-w] -f <solverLogFile>

Parse solver log file for PID and kill it.
Optinally send write signal instead (-w)

OPTIONS:
    -h Show this help message
    -f <logFile> Specify logfile corresponding to run
    -w Do not kill, force a write now!

EOF
}

KILL_SIG=''
LOG_FILE=''
while getopts ":hwf:" opt; do
    case $opt in
        h)
            help
            exit 0
        ;;
        f)
            LOG_FILE=$OPTARG
        ;;
        w)
            DO_WRITE='y'
            KILL_SIG='-10'
        ;;
        \?)
            echo -e "Invalid option -$OPTARG" >&2
            exit 1
        ;;
        :)
            echo "Option -$OPTARG requires an argument." >&2
            exit 1
        ;;
    esac
done

if [ -f "$LOG_FILE" ]; then
    PID=$(awk '$1~"PID" {print $3}' $LOG_FILE)
    kill $KILL_SIG $PID
else
    echo -e "Log file $LOG_FILE not found"
    help
    exit 1
fi


