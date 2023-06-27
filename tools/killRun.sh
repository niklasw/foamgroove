#!/bin/bash
#
# Parse solver log file for PID and kill it.
# Optionally send write signal instead.

prog=$(basename $0)

function help()
{
    cat << EOF
Usage: $prog [-w] [-f] <solverLogFile>

Parse solver log file for PID and kill it.
Optinally send write signal instead (-w)

OPTIONS:
    -h Show this help message
    -f force
    -w Do not kill, force a write now

EOF
}

KILL_SIG='-s SIGTERM'
LOG_FILE=''
while getopts ":hwf" opt; do
    case $opt in
        h)
            help
            exit 0
        ;;
        f)
            KILL_SIG='-9'
        ;;
        w)
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

shift $((OPTIND - 1))

if [ -z "$LOG_FILE" ]; then
    LOG_FILE=$1
fi

if [ -f "$LOG_FILE" ]; then
    PID=$(awk '$1~"PID" {print $3}' $LOG_FILE)
    echo -e "Trying \"kill $KILL_SIG $PID\""
    exec kill $KILL_SIG $PID
else
    echo -e "Log file $LOG_FILE not found"
    help
    exit 1
fi
