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
    createKinematicCloudPositions

Description

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "rhoThermo.H"
#include "RASModel.H"
#include "Random.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    timeSelector::addOptions();
    argList::addOption("cellFraction", "cellFraction", "Fraction of cells that will get a particle");
    #include "setRootCase.H"
    #include "createTime.H"
    #include "createMesh.H"

    scalar particleToCellFraction = 0.1;
    args.optionReadIfPresent("cellFraction",particleToCellFraction);

    //- Put one parcel in each cell centre
    tmp<vectorField> ccPtr = mesh.C().internalField();
    const vectorField& cc = ccPtr();

    List<vector> positionList(mesh.C().size());
    label i=0;
    forAll ( cc, I)
    {
        Random r(I);
        if ( r.scalar01() < particleToCellFraction )
        {
            positionList[i] = cc[I];
            i++;
        }
    }


    IOobject particlesHdr
    (
        "kinematicCloudPositions",
        runTime.constant(),
        runTime,
        IOobject::NO_READ,
        IOobject::NO_WRITE
    );

    positionList.resize(i);

    IOField<vector> particles(particlesHdr, vectorField(positionList));

    particles.write();

    Info << "End\n" << endl;

    return 0;
}


// ************************************************************************* //
