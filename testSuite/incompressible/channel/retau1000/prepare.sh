#!/usr/bin/env sh

function relocateMesh()
{
    rm -rf constant/polyMesh && mv $1/polyMesh constant/.
    rm -rf $1
}

function createPatchesOld()
{
    surfaceToPatch gmsh/boundary.obj
    relocateMesh 1
    createPatch -overwrite
}

function createPatches()
{
    topoSet
    createPatch -overwrite
}

function extrude()
{
    foamDictionary -entry surface -set "\"$1\"" system/extrudeMeshDict
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
    extrude $surfaceFile
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
    createPatches
    polyDualMesh -overwrite -doNotPreserveFaceZones 75
}

[[ -d constant/polyMesh ]] && rm -r constant/polyMesh

eval $__meshtype
cp -r zero 0

