/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2013 OpenFOAM Foundation
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
    createTurbulentK

Description
    Creates a full set of turbulence fields.

    - Currently does not output nut and nuTilda

Source files:
    createFields.H

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "incompressible/singlePhaseTransportModel/singlePhaseTransportModel.H"
#include "LESModel.H"
#include "calculatedFvPatchFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    timeSelector::addOptions();

    #include "setRootCase.H"
    #include "createTime.H"

    instantList timeDirs = timeSelector::select0(runTime, args);

    #include "createMesh.H"
    #include "createFields.H"

    forAll(timeDirs, timeI)
    {
        runTime.setTime(timeDirs[timeI], timeI);

        Info<< "Time = " << runTime.timeName() << endl;

        // Cache the turbulence fields

        //const volScalarField k(LESModel->k());

        /*
        Info<< "\nRetrieving field epsilon from turbulence model" << endl;
        const volScalarField epsilon(LESModel->epsilon());

        Info<< "\nRetrieving field R from turbulence model" << endl;
        const volSymmTensorField R(LESModel->R());
        */

        // Check availability of tubulence fields

        if (IOobject("k", runTime.timeName(), mesh).headerOk())
        {
            Info<< "\nRetrieving field k from turbulence model" << endl;
            Info<< "\nWriting turbulence field k" << endl;
            k.internalField() = LESModel->k()().internalField();
            k.write();
        }
        else
        {
            Info<< "\nTurbulence k field already exists" << endl;
        }
        if (!IOobject("nuEff", runTime.timeName(), mesh).headerOk())
        {
            volScalarField nuEff
            (
                IOobject
                (
                    "nuEff",
                    runTime.timeName(),
                    mesh,
                    IOobject::NO_READ,
                    IOobject::NO_WRITE
                ),
                mesh,
                dimensionedScalar("nut", LESModel->nuEff()->dimensions(),1e-6),
                zeroGradientFvPatchField<scalar>::typeName
            );
            nuEff.internalField() = LESModel->nuEff()().internalField();
            nuEff.write();
        }

        /*
        if (!IOobject("epsilon", runTime.timeName(), mesh).headerOk())
        {
            Info<< "\nWriting turbulence field epsilon" << endl;
            epsilon.write();
        }
        else
        {
            Info<< "\nTurbulence epsilon field already exists" << endl;
        }

        if (!IOobject("R", runTime.timeName(), mesh).headerOk())
        {
            Info<< "\nWriting turbulence field R" << endl;
            R.write();
        }
        else
        {
            Info<< "\nTurbulence R field already exists" << endl;
        }

        if (!IOobject("omega", runTime.timeName(), mesh).headerOk())
        {
            const scalar Cmu = 0.09;

            Info<< "creating omega" << endl;
            volScalarField omega
            (
                IOobject
                (
                    "omega",
                    runTime.timeName(),
                    mesh
                ),
                epsilon/(Cmu*k),
                epsilon.boundaryField().types()
            );
            Info<< "\nWriting turbulence field omega" << endl;
            omega.write();
        }
        else
        {
            Info<< "\nTurbulence omega field already exists" << endl;
        }
        */
    }

    Info<< "\nEnd\n" << endl;

    return 0;
}


// ************************************************************************* //

