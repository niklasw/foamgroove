#!/usr/bin/env sh

newTempCase()
{
    [[ -d $1 ]] && rm -rf $1 ;
    mkdir $1;
    ( touch $1/case.foam )
    cp -r constant system $1/.
}

function relocateMesh()
{
    rm -rf constant/polyMesh && mv $1/polyMesh constant/.
    rm -rf $1
}

function createPatches()
{
    topoSet
    createPatch -overwrite
}

function extrude()
{
    (
        set -x
        cd system
        ln -sf $1 extrudeMeshDict
    )
    foamDictionary -entry surface -set "\"$2\"" system/extrudeMeshDict
    extrudeMesh
    mirrorMesh -overwrite
}

extrudePatch()
{
    (
        set -x
        cd system
        cat extrudeMeshDict > extrudeMeshDict.org
        ln -sf $1 extrudeMeshDict
    )
    extrudeMesh
    mirrorMesh -overwrite
}

function createMesh()
{
    gmshFile=$1
    surfaceFile=$2
    shift 2
    gmshArgs=$@
    gmsh $gmshFile -2 $gmshArgs -o $surfaceFile
    extrude extrudeMeshDict.surface $surfaceFile
    createPatches
}

function tri()
{
    createMesh gmsh/plane_tri.geo gmsh/bottom.vtk -smooth 3
#    refineWallLayer -overwrite "(wallBottom wallTop)" 0.5
}

function quad()
{
    createMesh  gmsh/plane_quad.geo gmsh/bottom.vtk
#    refineWallLayer -overwrite "(wallBottom wallTop)" 0.5
}

function quadtri()
{
    createMesh gmsh/plane_quad.geo gmsh/bottom.stl
#    refineWallLayer -overwrite "(wallBottom wallTop)" 0.5
}

function poly()
{
    gmsh gmsh/box_tet.geo -3 -smooth 3 -o gmsh/volume.msh
    gmshToFoam gmsh/volume.msh
    polyDualMesh -overwrite -doNotPreserveFaceZones 75
    combinePatchFaces  -overwrite 75
    createPatches
    newTempCase tmpCase
    extrudePatch extrudeMeshDict.patch
    createPatches
}

[[ -d constant/polyMesh ]] && rm -r constant/polyMesh

eval $__meshtype
cp -r zero 0

