// blockMesh :  Block mesh description file
/*--------------------------------*- C++ -*----------------------------------*\
| =========                 |                                                 |
| \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox           |
|  \\    /   O peration     | Version:  4.x                                   |
|   \\  /    A nd           | Web:      www.OpenFOAM.org                      |
|    \\/     M anipulation  |                                                 |
\*---------------------------------------------------------------------------*/
FoamFile
{
    version     2.0;
    format      ascii;
    class       dictionary;
    location    "system";
    object      blockMeshDict;
}
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

changecom(//)changequote([,])
divert(-1)dnl

define(notusedcalc, [esyscmd(perl -e 'printf ($1)')])
define(calc, [esyscmd(python -c 'from math import *; import sys; sys.stdout.write(str($1))')])
define(icalc, [esyscmd(python -c 'from math import *; import sys; sys.stdout.write(str(int(round($1))))')])

define(VCOUNT, 0)
define(vlabel, [[// ]Vertex $1 = VCOUNT define($1, VCOUNT)define([VCOUNT], incr(VCOUNT))])


define(R,0.08)
define(R0,0.005)
define(XMIN,-0.4)
define(XMAX,0.1)
define(angle,10)
define(baseSize,0.001)

define(L,calc(XMAX-XMIN))
define(PI,3.141592653589793)
define(toRad,calc(PI/180))
define(NX,icalc((XMAX-XMIN)/baseSize))
define(NY,icalc(R/baseSize))
define(NZ,2))
//define(NZ,icalc((R+R0)*0.5*angle*toRad/baseSize))

define(ALPHA,calc(toRad*angle/2.0))


define(ZMANT, calc(R*sin(ALPHA))) // Mantle Z coordinate
define(YMANT, calc(R*cos(ALPHA))) // Mantle Y coordinate
define(ZAXIS, calc(R0*sin(ALPHA)))// Axis Z coordinate
define(YAXIS, calc(R0*cos(ALPHA)))// Axis Y coordinate
divert(0)dnl

convertToMeters 1.0;

vertices
(
    (XMIN  YAXIS  ZAXIS ) vlabel(axisMinRight)
    (XMIN  YMANT  ZMANT ) vlabel(mantMinRight)
    (XMIN  YMANT -ZMANT ) vlabel(mantMinLeft)
    (XMIN  YAXIS -ZAXIS ) vlabel(axisMinLeft)

    (XMAX YAXIS  ZAXIS ) vlabel(axisMaxRight)
    (XMAX YMANT  ZMANT ) vlabel(mantMaxRight)
    (XMAX YMANT -ZMANT ) vlabel(mantMaxLeft)
    (XMAX YAXIS -ZAXIS ) vlabel(axisMaxLeft)
);

blocks
(
    hex (axisMinLeft  axisMaxLeft  mantMaxLeft  mantMinLeft
         axisMinRight axisMaxRight mantMaxRight mantMinRight)
    (NX NY NZ)
    simpleGrading (1 1 1)
);

edges
(
    arc mantMinRight    mantMinLeft    (XMIN R 0)
    arc mantMaxLeft     mantMaxRight   (XMAX R 0)

    arc axisMinRight    axisMinLeft    (XMIN R0 0)
    arc axisMaxLeft     axisMaxRight   (XMAX R0 0)
);

boundary
(
    outflow
    {
        type    patch;
        faces
        (
            (axisMinRight mantMinRight mantMinLeft axisMinLeft)
        );
    }

    inflow
    {
        type    patch;
        faces
        (
            (axisMaxRight mantMaxRight mantMaxLeft axisMaxLeft)
        );
    }
    axis
    {
        type    wall;
        inGroups (slipGroup);
        faces
        (
            (axisMinRight axisMinLeft axisMaxLeft axisMaxRight)
        );
    }
    mantle
    {
        type    wall;
        faces
        (
            (mantMinRight mantMaxRight mantMaxLeft mantMinLeft)
        );
    }
    left
    {
        type    symmetryPlane;
        faces
        (
            (axisMinLeft mantMinLeft mantMaxLeft axisMaxLeft)
        );
    }
    right
    {
        type    symmetryPlane;
        faces
        (
            (axisMinRight axisMaxRight mantMaxRight mantMinRight)
        );
    }
);


