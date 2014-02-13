#!/bin/bash

#- Just a wrapper that informs and pipes stdout to log.
runme()
{
    echo -e "Running: $@"
    [[ -d logs ]] || mkdir logs
    [[ "$1" == "mpirun" ]] &&  log=$4 || log=$1
    $@ > logs/$log.log 2>&1
    if [ $? != "0" ]; then
        cat logs/$log.log
        echo -e "Some error! Exiting."
        exit 1
    fi
}

#- Wrapper around runme, selecting serial or parallel run
runmePar()
{
    np=$1
    shift 1
    if [ "$np" == "1" ]; then
        runme $@
    else
        runme mpirun -np $np $@ -parallel
    fi
}

#- Replaces processor*/0 with serial 0, to keep templates
distributeZero()
{
    for p in processor*; do
        rm -rf $p/0
        cp -rv 0 $p/0
    done
}

NP=1
if [ -z $1 ]; then
    read -p "How many processes? " NP
fi

foamClearPolyMesh

runmePar 1 blockMesh -dict system/blockMeshDict

runmePar 1 changeDictionary

runmePar 1 decomposePar -force

runmePar $NP snappyHexMesh -overwrite

runmePar $NP changeDictionaryLight

runmePar 1 distributeZero

