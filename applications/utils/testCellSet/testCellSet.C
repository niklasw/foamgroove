/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 1991-2005 OpenCFD Ltd.
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

Application
    testCellSet

Description
    Just a test application

\*---------------------------------------------------------------------------*/

#include "Time.H"
#include "polyMesh.H"
#include "argList.H"
#include "IStringStream.H"
#include "fvCFD.H"
#include "error.H"
#include "boundBox.H"
#include "cellSet.H"


using namespace Foam;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
//  Main program:

int main(int argc, char *argv[])
{
    timeSelector::addOptions();
    //argList::noParallel();
    argList::validOptions.insert("cellSet", "cellSet");
#   include "setRootCase.H"
#   include "createTime.H"
#   include "createMesh.H"

    word cellSetName;
    bool entireMesh = true;

    if ( args.options().found("cellSet") )
    {
        cellSetName = args.options()["cellSet"];
        entireMesh = false;
        Info << "Operating on cellSet: " << cellSetName << "\n" << endl;
    }

    instantList timeDirs = timeSelector::select0(runTime, args);

    forAll (timeDirs, timeI)
    {
        runTime.setTime(timeDirs[timeI], timeI);

        double sumVolume = 0;
        double entireVolume = 0;
        if ( entireMesh )
        {
            forAll (mesh.V(), i)
            {
                sumVolume += mesh.V()[i];
            }

            reduce(sumVolume,sumOp<scalar>());
        }
        else
        {

            cellSet cSet(mesh,cellSetName);

            forAllConstIter(labelHashSet, cSet, iter)
            {
                sumVolume += mesh.V()[iter.key()];
            }

            reduce(sumVolume,sumOp<scalar>());


        }
        Info << "CellSet Volume = " << sumVolume << endl;
        Info << "Mesh Volume = " << gSum(mesh.V()) << endl;


    }

    return(0);
}


// ************************************************************************* //
