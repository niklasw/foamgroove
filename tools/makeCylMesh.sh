#!/bin/bash

NP=${1:-1}

subCase=bulletMesh
tempCase=tmp0

source /remote/soft/OpenFOAM/foamGroove/tools/meshFunctions.sh

# ---------------------------------------------------------

rm -rf $subCase
newTempCase $subCase

# Create the ambience mesh
newBlock blockMeshDict
setDecomposition $NP
runme decomposePar -force
runmePar $NP snappyHexMesh -overwrite
runmePar $NP renumberMesh -overwrite
runme reconstructParMesh -constant
runme rm -rf processor*
newTempCase $tempCase
extrude extrudeMeshDict_wedgeExt
runme rm -rf tmp*

runme createPatch -overwrite

runme changeDictionary

# Now, create the barrel internal mesh.
(
    cd $subCase
    newBlock blockMeshDict_$subCase
    newTempCase $tempCase
    extrude extrudeMeshDict_wedgeInt
    runme rm -rf $tempCase

    batchFile=/tmp/setSet.$$

    echo "cellSet internal new boxToCell (-100 -100 -100) (100 100 100)" > $batchFile
    runme setSet -constant -noVTK -batch $batchFile

    runme setsToZones -noFlipMap
    rm -f $batchFile
)

# Merge external mesh with internal mesh and clear subCase (internal)
merge $PWD $PWD/$subCase
runme rm -rf $subCase

# Clean out empty patches, if any...
runme createPatch -overwrite -dict system/createPatchDict_cleanup

# Do all the funny ACMI couple interface patches stuff
runme topoSet -dict system/topoSetDict_baffles
runme createBaffles -overwrite

# Clean out empty patches, if any...
runme createPatch -overwrite -dict system/createPatchDict_cleanup

runme checkMesh

