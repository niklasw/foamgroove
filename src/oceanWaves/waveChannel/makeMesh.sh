#!/bin/bash

function refine()
{
    topoSet -dict system/topoSetDict$1
    refineMesh -overwrite -dict system/refineMeshDict$1
}

foamClearPolyMesh

blockMesh -dict system/blockMeshDict

refine 0
refine 1

changeDictionary

checkMesh |egrep "cells:|Mesh OK"

rm -r 0
cp -r zero 0

