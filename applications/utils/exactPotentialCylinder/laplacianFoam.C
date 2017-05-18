/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2015 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Application
    laplacianFoam

Description
    Solves a simple Laplace equation, e.g. for thermal diffusion in a solid.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "simpleControl.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::addOption
    (
        "radius",
        "scalar",
        "Cylinder radius"
    );

    argList::addOption
    (
        "centre",
        "vector",
        "Cylinder centre"
    );

    argList::addOption
    (
        "axis",
        "vector",
        "Cylinder axis vector"
    );

    argList::addOption
    (
        "flowDirection",
        "vector",
        "Flow vector"
    );

    #include "setRootCase.H"
    #include "createTime.H"

    scalar cylRadius = 0.5;
    args.optionReadIfPresent("radius",cylRadius);
    vector cylCentre(0,0,0);
    args.optionReadIfPresent("centre",cylCentre);
    vector cylAxis(0,1,0);
    args.optionReadIfPresent("axis",cylAxis);
    vector flowVector(1,0,0);
    args.optionReadIfPresent("flowDirection",flowVector);
    cylAxis /= mag(cylAxis);
    flowVector /= mag(flowVector);
    const scalar cylDiameter = 2*cylRadius;

    #include "createMesh.H"
    #include "createFields.H"
    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
