#!/usr/bin/env sh

rm -r *.obj *.log 2>/dev/null
foamCleanPolyMesh
rm gmsh/*.vtk gmsh/*.stl 2>/dev/null
rm -rf subCase_*
