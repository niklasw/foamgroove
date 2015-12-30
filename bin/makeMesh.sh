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

setDecomposition()
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

#- Replaces processor*/0 with serial 0, to keep templates
distributeZero()
{
    [[ -d processor0 ]] || return 0

    if [[ "$1" == "" ]]; then
        sourceDir=0
    else
        sourceDir=$1

    for p in processor*; do
        rm -rf $p/0
        cp -rv 0 $p/0
    done
}

checkLocalMakeMesh()
{
    if [ -f makeMesh.sh ]; then
        echo -e "Local makeMesh.sh found!"
        read -p "Do you want to run that instead? [Y/n] " ans
        ans="${ans:=Y}"
        if [[ "$ans" == "Y" ]]; then
            echo -e "Running local makeMesh.sh"
            ./makeMesh.sh
            exit 0
        fi
        echo -e "Continuing with general makeMesh.sh then"
    fi
}

checkLocalMakeMesh

NP=${1:-1}

if [ -z $1 ]; then
    read -p "How many processes? [1] " NP
    NP="${NP:=1}"
fi

foamClearPolyMesh

runme blockMesh -dict system/blockMeshDict

runme changeDictionary

runme setDecomposition $NP

[[ "$NP" == "1" ]] ||  runme decomposePar -force

runmePar $NP snappyHexMesh -overwrite

runmePar $NP changeDictionaryLight

runmePar $NP checkMesh

runme distributeZero zero

