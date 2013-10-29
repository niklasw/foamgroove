/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011 OpenFOAM Foundation
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
    writePatchPointLabels

Description
    Writes to file patch point labelLists for patches given as argument list.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "volFields.H"
#include "IOobjectList.H"
//#include "SortableList.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::noParallel();
    argList::addOption("patches","patches","Word list of patch names");

#   include "setRootCase.H"
#   include "createTime.H"
#   include "createMesh.H"

    List<word> patchNames;
    if (args.options().found("patches"))
    {
        IStringStream(args.options()["patches"])() >> patchNames;
    }
    List<label> patchLabels(patchNames.size(),-1);

    forAll(patchNames, i)
    {
        word patchName = patchNames[i];
        label patchLabel = mesh.boundaryMesh().findPatchID(patchName);
        if (patchLabel < 0)
        {
            FatalErrorIn(args.executable())
                << "Patch name not found in case: "
                << patchName
                << exit(FatalError);
        }
        else
        {
            patchLabels[i] = patchLabel;
        }
    }

    forAll(patchNames, i)
    {
        const polyPatch& patch = mesh.boundaryMesh()[patchLabels[i]];

        Info << "Writing patch point labels for patch "
             << patch.name() << "\n" << endl;

        IOobject patchLabelsHdr
        (
            "labels_"+patch.name(),
            runTime.constant(),
            "structuralModes",
            runTime,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        );

        labelIOList patchPointLabels
        (
            patchLabelsHdr,
            patch.meshPoints()
        );

        patchPointLabels.write();
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
