#!/bin/bash

function nProcs()
{
    dPD=system/decomposeParDict
    [[ -f $dPD ]] || exit 1
    np=$(awk '$1~"numberOfSubdomains" && sub(";","",$2) {print $2}' $dPD)
    echo $np
}

function runApp()
{
    if [[ $1 == "-p" ]]; then
        app=$2
        NP=$(nProcs)
        echo "Running $app on $NP processes"
        shift 2
        mpirun -np $NP $app -parallel "$@" > $app.log 2>&1
    else
        app=$1
        echo "Running $app on 1 process"
        shift 1
        $app "$@" > $app.log 2>&1
    fi
}

runApp renumberMesh -overwrite
runApp decomposePar -force
runApp -p setFields
runApp -p interFoam
