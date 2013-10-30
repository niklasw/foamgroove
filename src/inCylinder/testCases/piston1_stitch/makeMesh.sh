#!/bin/bash

#- Just a wrapper that informs and pipes stdout to log.
runme()
{
    echo -e "Running: $@"
    [[ -d logs ]] || mkdir logs
    $@ > logs/$1.log
    if [ $? != "0" ]; then
        cat logs/$1.log
        echo -e "Some error! Exiting."
        exit 1
    fi
}

#- Simple (not safe) function to edit one line entries
#  on the form "entryName  value;"
changeEntry()
{
    sed -i.bak "s%\($2\s*\).*;%\1$3;%" $1
}

#- Clones the case to a sub-folder, carry the essentials
newTempCase()
{
    [[ -d $1 ]] && runme rm -rf $1 ;
    runme mkdir $1;
    ( touch $1/case.foam )
    runme cp -r setup.OpenFOAM constant system $1/.
}

#- Create a new blockMesh from selected dictionary
#  and run snappy on that block
newBlock()
{
    runme foamClearPolyMesh
    runme blockMesh -dict system/$1
    runme snappyHexMesh -overwrite
}


#- Links to the correct extrudeMeshDict and executes
extrude()
{
    ( set -x; cd system; ln -sf $1 extrudeMeshDict )
    runme extrudeMesh

}

#- Merge two meshes and try to stitch them
merge() { runme mergeMeshes $1 $2 -overwrite; }
stitch() { runme stitchMesh $1 $2 -case $3 -overwrite; }
mergeAndStitch()
{
    merge $1 $2
    stitch $3 $4 $1
}

#- Move the mesh from a sub-case to this case. Clear
#  this case current mesh first.
getMesh()
{
    runme foamClearPolyMesh
    runme mv $1/constant/polyMesh/* constant/polyMesh/.
}

# --------------------------------------------------------- 

changeEntry system/snappyHexMeshDict locationInMesh '(-0.015 -0.0025 -0.016312)'
newBlock blockMeshDict0
newTempCase tmp0
extrude extrudeMeshDict_wedge0
newTempCase tmpWedge0

changeEntry system/snappyHexMeshDict locationInMesh '(-0.031 -0.0025 -0.016312)'
newBlock blockMeshDict1
newTempCase tmp1
extrude extrudeMeshDict_wedge1
newTempCase tmpWedge1

changeEntry system/snappyHexMeshDict locationInMesh '(-0.051 -0.0025 -0.016312)'
newBlock blockMeshDict2
newTempCase tmp2
extrude extrudeMeshDict_wedge2
newTempCase tmpWedge2

mergeAndStitch tmpWedge1 tmpWedge2 linerWall_1 center_2
mergeAndStitch tmpWedge0 tmpWedge1 linerWall_0 center_1

runme createPatch -overwrite -case tmpWedge0

getMesh tmpWedge0

extrude extrudeMeshDict_cylinder

runme rm -rf tmp*
