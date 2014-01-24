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

runmePar()
{
    np=$1
    shift 1
    runme mpirun -np $np $@ -parallel
}

NP=4

foamClearPolyMesh

runme blockMesh -dict system/blockMeshDict

runme changeDictionary

runme decomposePar -force

runmePar $NP snappyHexMesh -overwrite

runmePar $NP changeDictionaryLight

distributeZero.sh



