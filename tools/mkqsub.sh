#!/bin/bash
#PBS -S /bin/bash
#PBS -N m4_pimple
#PBS -l nodes=2:ppn=48
#PBS -l walltime=2:00:00
#PBS -m be
#PBS -q rosso
#PBS -o meshJob
#PBS -l other=e26504

source /home/soft/OpenFOAM/etc/bashrc-2.3.x

function mkHostFile()
{
    pbsFile=$1
    hostFile=$2
    nFreeCores=${3:-0} #Use argument 3 if it is given, else 0
    cat $pbsFile | sort | uniq -c \
        | awk -v x=$nFreeCores '{printf("%s slots=%i\n",$2,$1-x)}' \
        > $hostFile
}

function setDecomposition()
{
    dict="system/decomposeParDict"
    if [ "$1" == "1" ]; then
        return 0
    elif [ ! -f $dict ]; then
        echo "Error: Missing decomposeParDict"
        exit 1
    fi
    sed -i -e "s%^\s*\(numberOfSubdomains\) .*;%\1 $1;%g" $dict
}

function runpar()
{
    ssh $(hostname) "cd $PBS_O_WORKDIR; mpirun -np $NCORES --machinefile $HOSTS $@ -parallel >& $LOGS/$1.log"
}

function runserial()
{
    $@ >& $LOGS/$1.log
}

LEAVE_CPUS=0
LOGS="$PBS_O_WORKDIR/logs"
HOSTS="$PBS_O_WORKDIR/machinefile"
NCORES=$(( PBS_NUM_NODES*(PBS_NUM_PPN-LEAVE_CPUS) ))

cd $PBS_O_WORKDIR

mkHostFile "$PBS_NODEFILE" "$HOSTS" "$LEAVE_CPUS"



