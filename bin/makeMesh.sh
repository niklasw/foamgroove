#!/bin/bash

STL_FILE="cubeWBoundaries"
locationInMesh="(0.02 0.01 0)"

./MeshScripts/stlToBoundingBlockMesh.py constant/triSurface/$STL_FILE.stl
cat blockMeshDict > constant/polyMesh/blockMeshDict
blockMesh

./MeshScripts/stlToSnappyRegions.py -f constant/triSurface/$STL_FILE.stl -o system/snappyHexMeshDict
sed -i -e "/features/,/)/ s%//%%; s/someLine.eMesh/$STL_FILE.eMesh/" system/snappyHexMeshDict
sed -i -e "s/locationInMesh.*;/locationInMesh $locationInMesh;/" system/snappyHexMeshDict

sed -i -e "s/\(^ *method *\).*;/\1hierarchical;/" system/decomposeParDict

vim system/snappyHexMeshDict +/refinementSurfaces

surfaceFeatureExtract -includedAngle 80 constant/triSurface/$STL_FILE.stl $STL_FILE
snappyHexMesh -overwrite
changeDictionary

sed -i -e "s/\(^ *method *\).*;/\1scotch;/" system/decomposeParDict



