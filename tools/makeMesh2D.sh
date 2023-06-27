#!/bin/bash

source meshFunctions.sh

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

newTempCase tempCase

runme extrudeMesh extrudeMeshDict.2d

runme createPatch -dict system/createPatchDict.empty -overwrite

runmePar $NP changeDictionary

runmePar $NP checkMesh

runme distributeZero zero

