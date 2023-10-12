#!/bin/sh
foamClearPolyMesh
rm -r 0
mkdir 0
cp 0.org/p 0/.
blockMesh
topoSet
subsetMesh -overwrite c0 -patch movingBlock
cp 0.org/pointDisplacement 0/.
setFields
