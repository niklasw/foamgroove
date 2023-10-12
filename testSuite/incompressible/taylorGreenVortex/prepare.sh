#!/usr/bin/env sh

function hex()
{
    blockMesh -dict system/blockMeshDict
}

function tet()
{
    gmsh -3 -optimize gmsh/gmsh_cube.geo
    gmshToFoam gmsh/gmsh_cube.msh
    renumberMesh -overwrite
    changeDictionary -dict system/changeDictionaryDict.gmsh
}

function poly()
{
    tet
    polyDualMesh 75 -overwrite
}

rm -rf 0
$__meshtype
cp -r zero 0
setupTaylorGreen
