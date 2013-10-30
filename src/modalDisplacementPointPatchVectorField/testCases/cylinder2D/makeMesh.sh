#!/bin/bash

foamClearPolyMesh
rm -rf tmpCase

blockMesh -dict system/blockMeshDict

snappyHexMesh -overwrite

createPatch -dict system/createPatchDict.cleanup -overwrite

mkdir tmpCase

cp -r constant system setup.OpenFOAM tmpCase/.

foamClearPolyMesh

extrudeMesh

createPatch -dict system/createPatchDict.cleanup -overwrite

changeDictionary

rm -rf tmpCase
