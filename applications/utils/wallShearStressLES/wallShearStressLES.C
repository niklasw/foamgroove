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
    wallShearStressLES

Description
    Calculates and reports wall shear stress for all patches, for the
    specified times when using LES turbulence models.

    Default behaviour assumes operating in incompressible mode.
    Use the -compressible option for compressible LES cases.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"

#include "incompressible/singlePhaseTransportModel/singlePhaseTransportModel.H"
#include "incompressible/LES/LESModel/LESModel.H"

#include "basicThermo.H"
#include "compressible/LES/LESModel/LESModel.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

void calcIncompressible
(
    const fvMesh& mesh,
    const Time& runTime,
    const volVectorField& U,
    volVectorField& wallShearStressLES
)
{
    #include "createPhi.H"

    singlePhaseTransportModel laminarTransport(U, phi);

    autoPtr<incompressible::LESModel> model
    (
        incompressible::LESModel::New(U, phi, laminarTransport)
    );

    const volSymmTensorField Reff(model->devReff());

    forAll(wallShearStressLES.boundaryField(), patchI)
    {
        wallShearStressLES.boundaryField()[patchI] =
        (
           -mesh.Sf().boundaryField()[patchI]
           /mesh.magSf().boundaryField()[patchI]
        ) & Reff.boundaryField()[patchI];
    }
}


void calcCompressible
(
    const fvMesh& mesh,
    const Time& runTime,
    const volVectorField& U,
    volVectorField& wallShearStressLES
)
{
    IOobject rhoHeader
    (
        "rho",
        runTime.timeName(),
        mesh,
        IOobject::MUST_READ,
        IOobject::NO_WRITE
    );

    if (!rhoHeader.headerOk())
    {
        Info<< "    no rho field" << endl;
        return;
    }

    Info<< "Reading field rho\n" << endl;
    volScalarField rho(rhoHeader, mesh);

    #include "compressibleCreatePhi.H"

    autoPtr<basicThermo> pThermo(basicThermo::New(mesh));
    basicThermo& thermo = pThermo();

    autoPtr<compressible::LESModel> model
    (
        compressible::LESModel::New
        (
            rho,
            U,
            phi,
            thermo
        )
    );

    const volSymmTensorField Reff(model->devRhoReff());

    forAll(wallShearStressLES.boundaryField(), patchI)
    {
        wallShearStressLES.boundaryField()[patchI] =
        (
           -mesh.Sf().boundaryField()[patchI]
           /mesh.magSf().boundaryField()[patchI]
        ) & Reff.boundaryField()[patchI];
    }
}


int main(int argc, char *argv[])
{
    timeSelector::addOptions();

    #include "addRegionOption.H"

    argList::addBoolOption
    (
        "compressible",
        "calculate compressible wall shear stress"
    );

    #include "setRootCase.H"
    #include "createTime.H"
    instantList timeDirs = timeSelector::select0(runTime, args);
    #include "createNamedMesh.H"

    const bool compressible = args.optionFound("compressible");

    forAll(timeDirs, timeI)
    {
        runTime.setTime(timeDirs[timeI], timeI);
        Info<< "Time = " << runTime.timeName() << endl;
        mesh.readUpdate();

        volVectorField wallShearStressLES
        (
            IOobject
            (
                "wallShearStressLES",
                runTime.timeName(),
                mesh,
                IOobject::NO_READ,
                IOobject::AUTO_WRITE
            ),
            mesh,
            dimensionedVector
            (
                "wallShearStressLES",
                sqr(dimLength)/sqr(dimTime),
                vector::zero
            )
        );

        IOobject UHeader
        (
            "U",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        );

        if (UHeader.headerOk())
        {
            Info<< "Reading field U\n" << endl;
            volVectorField U(UHeader, mesh);

            if (compressible)
            {
                calcCompressible(mesh, runTime, U, wallShearStressLES);
            }
            else
            {
                calcIncompressible(mesh, runTime, U, wallShearStressLES);
            }
        }
        else
        {
            Info<< "    no U field" << endl;
        }

        Info<< "Writing wall shear stress to field " << wallShearStressLES.name()
            << nl << endl;

        wallShearStressLES.write();
    }

    Info<< "End" << endl;

    return 0;
}


// ************************************************************************* //
