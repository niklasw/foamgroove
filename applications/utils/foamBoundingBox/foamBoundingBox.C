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
    foamBoundingBox

Description
    Scales the mesh points in the polyMesh directory by a factor supplied as an 
    argument.

\*---------------------------------------------------------------------------*/

#include "Time.H"
#include "polyMesh.H"
#include "argList.H"
#include "IStringStream.H"
#include "fvCFD.H"
#include "error.H"
#include "boundBox.H"


using namespace Foam;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
//  Main program:

int main(int argc, char *argv[])
{
    timeSelector::addOptions();
    //argList::noParallel();
    argList::validOptions.insert("region","region");
    argList::validOptions.insert("patch", "patchName");
    argList::validOptions.insert("cellSet", "cellSet");
#   include "setRootCase.H"
#   include "createTime.H"

    word patchName;
    bool entireMesh = true;

    if ( args.options().found("patch") )
    {
        patchName = args.options()["patch"];
        entireMesh = false;
        Info << "Operating on patch: " << patchName << "\n" << endl;
    }

    instantList timeDirs = timeSelector::select0(runTime, args);

    forAll (timeDirs, timeI)
    {
        runTime.setTime(timeDirs[timeI], timeI);
        pointField *pointsPtr;

        if ( entireMesh )
        {
            pointIOField points
            (
                IOobject
                (
                    "points",
                    runTime.findInstance(polyMesh::meshSubDir, "points"),
                    polyMesh::meshSubDir,
                    runTime,
                    IOobject::MUST_READ,
                    IOobject::NO_WRITE,
                    false
                )
            );

            pointsPtr = new pointField(points);
        }
        else
        {
#           include "createMeshNoClear.H"
            label patchID = mesh.boundaryMesh().findPatchID(patchName);
            if ( patchID < 0 )
            {
                FatalErrorIn("foamBoundingBox.C")
                   << "Cannot find a patch named " << patchName << abort(FatalError);
            }

            const labelList &patchPointLabels(mesh.boundaryMesh()[patchID].meshPoints());

            pointsPtr = new pointField(patchPointLabels.size());

            pointField &tmpPoints = *pointsPtr;
            forAll (patchPointLabels, i )
            {
                tmpPoints[i] = mesh.points()[patchPointLabels[i]];
            }

        }

        pointField &points = *pointsPtr;

        boundBox box(points);

        delete pointsPtr;

        Info<< "Bounding box: min = "
            << box.min() << " max = " << box.max() << " meters." << endl;
        Info<< "Bounding box centre = "
            << (box.max()-box.min())*0.5+box.min() << endl;

        Info<< "Bounding box dimensions = " << box.max() - box.min() << " meters." << endl;
    }

    return(0);
}


// ************************************************************************* //
