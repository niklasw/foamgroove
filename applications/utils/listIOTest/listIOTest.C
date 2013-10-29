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
    listIOTest

Description
    Creates, writes and reads a vectorListIOList.

Usage
    See -help
\*---------------------------------------------------------------------------*/

#include "argList.H"
#include "fvCFD.H"
#include "vectorListIOList.H"
#include "Time.H"
#include "mathematicalConstants.H"

using namespace Foam;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Function to generate test modes...
vectorField makeMode
(
    const vectorField& coords,
    const direction d,
    const label mode,
    const vector origin,
    const vector axis,
    const scalar amplitude
)
{
    scalar pi = constant::mathematical::pi;
    scalarField modeCoordinate = axis & (coords - origin);
    scalar modeLength = max(modeCoordinate);
    vector amp = vector::zero;
    amp.replace(d,amplitude);
    vectorField displacement = 0.5*amp*(1-cos(mode*pi*0.5*modeCoordinate/modeLength));
    return displacement;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::addOption
    (
        "listName",
        "word",
        "Name of list to read"
    );
    argList::addOption
    (
        "patchName",
        "word",
        "Name of moving patch"
    );
    argList::addOption
    (
        "nModes",
        "label",
        "Number of modes"
    );
#   include "setRootCase.H"
#   include "createTime.H"
#   include "createMesh.H"

    word listName;
    if ( ! args.optionReadIfPresent("listName", listName) )
    {
        FatalErrorIn(args.executable())
            << "List name must be specified" << exit(FatalError);
    }

    word patchName;
    if ( ! args.optionReadIfPresent("patchName", patchName) )
    {
        FatalErrorIn(args.executable())
            << "patch name must be specified" << exit(FatalError);
    }

    listName =listName+"_"+patchName;

    label nModes;
    if ( ! args.optionReadIfPresent("nModes", nModes) )
    {
        nModes = 1;
        WarningIn(args.executable())
            << "Number of modes not given. Defaults to 1" << endl;
    }

    label patchId = mesh.boundaryMesh().findPatchID(patchName);
    const polyPatch& patch = mesh.boundaryMesh()[patchId];

    const vectorField& pPointCoords = patch.localPoints();


    /*
     * Test create vectorListIOList from scratch
     * and write to file.
     */
    IOobject modeHdr0
    (
        listName,
        runTime.constant(),
        runTime,
        IOobject::NO_READ,
        IOobject::NO_WRITE
    );

    vectorListIOList modeList0
    (
        modeHdr0,
        nModes
    );

    forAll (modeList0, modeI)
    {
        modeList0[modeI] = makeMode
                           (
                                pPointCoords,       // const vectorField& coords,
                                vector::Y,          // const direction d,        
                                modeI,              // const label mode,         
                                vector(0,0.1,0),    // const vector origin,      
                                vector(1,0,0),      // const vector axis,        
                                0.02                // const scalar amplitude    
                           );
    }

    modeList0.write();

    /*
     * Test read same vectorListIOList
     * from file (and spit out to terminal).
     */
    IOobject modeHdr1
    (
        listName,
        runTime.constant(),
        runTime,
        IOobject::MUST_READ,
        IOobject::NO_WRITE
    );

    if ( ! modeHdr0.headerOk() )
    {
        FatalErrorIn(args.executable())
            << "Cannot read mode list file" << exit(FatalError);
    }

    vectorListIOList modeList1
    (
        modeHdr1
    );

    Info << modeList1 << endl;

    Info << "\nEnd" << endl;
    return 0;
}


// ************************************************************************* //
