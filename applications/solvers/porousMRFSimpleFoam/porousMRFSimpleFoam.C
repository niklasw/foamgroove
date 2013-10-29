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
    porousMRFSimpleFoam

Description
    Steady-state solver for incompressible, turbulent flow with
    implicit or explicit porosity treatment and MRF regions.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "singlePhaseTransportModel.H"
#include "RASModel.H"
#include "porousZones.H"
#include "MRFZones.H"
#include "simpleControl.H"
#include "IObasicSourceList.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

void systemPressureDrop(const fvMesh& mesh, const dimensionedScalar rho)
{
    label inletID = -1;
    label outletID = -1;
    forAll ( mesh.boundaryMesh().names(), nameI )
    {
        word patchName = mesh.boundaryMesh().names()[nameI];
        if ( regExp("inlet",true).search(patchName) )
        {
            inletID = mesh.boundaryMesh().findPatchID(patchName);
        }
        if ( regExp("outlet",true).search(patchName) )
        {
            outletID = mesh.boundaryMesh().findPatchID(patchName);
        }
    }

    if ( (inletID >= 0) && (outletID >= 0) )
    {
        //const fvsPatchVectorField& inletSf = mesh.Sf().boundaryField()[inletID];
        //const fvsPatchVectorField& outletSf = mesh.Sf().boundaryField()[outletID];
        const fvsPatchScalarField& inletMagSf = mesh.magSf().boundaryField()[inletID];
        const fvsPatchScalarField& outletMagSf = mesh.magSf().boundaryField()[outletID];

        double inletArea = gSum(inletMagSf);
        double outletArea= gSum(outletMagSf);

        const volScalarField& p = mesh.lookupObject<volScalarField>("p");
        const volVectorField& U = mesh.lookupObject<volVectorField>("U");
        const surfaceScalarField& phi = mesh.lookupObject<surfaceScalarField>("phi");

        const fvPatchScalarField& pIn = p.boundaryField()[inletID];
        const fvsPatchScalarField& phiIn = phi.boundaryField()[inletID];
        const fvPatchVectorField& UIn = U.boundaryField()[inletID];

        const fvPatchScalarField& pOut = p.boundaryField()[outletID];
        const fvsPatchScalarField& phiOut = phi.boundaryField()[outletID];
        const fvPatchVectorField& UOut = U.boundaryField()[outletID];

        scalarField pTotIn = 0.5*rho.value()*magSqr(UIn) + pIn;
        scalar pTotMfaIn = gSum(phiIn*pTotIn)/gSum(SMALL+phiIn);

        scalarField pTotOut = 0.5*rho.value()*magSqr(UOut) + pOut;
        scalar pTotMfaOut = gSum(phiOut*pTotOut)/gSum(SMALL+phiOut);

        Info << "Surface average static pressure Inlet / Outlet = "
             << gSum(inletMagSf*pIn)/inletArea*rho.value() << " / "
             << gSum(outletMagSf*pOut)/outletArea*rho.value() << endl;

        Info << "Monitor TotalPressureLoss  = " << pTotMfaIn - pTotMfaOut << nl << endl;
    }
};

int main(int argc, char *argv[])
{
    #include "setRootCase.H"
    #include "createTime.H"
    #include "createMesh.H"
    #include "createFields.H"
    #include "initContinuityErrs.H"

    simpleControl simple(mesh);

    MRFZones mrfZones(mesh);
    mrfZones.correctBoundaryVelocity(U);

    dimensionedScalar rho = laminarTransport.lookupOrDefault("rho",dimensionedScalar("rho",dimMass/dimVol,1.0) );
    Info << "Density for pressure measurement = " << rho.value() << endl;

    #include "createPorousZones.H"

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    Info<< "\nStarting time loop\n" << endl;

    while (simple.loop())
    {
        Info<< "Time = " << runTime.timeName() << nl << endl;

        // Pressure-velocity SIMPLE corrector
        {
            #include "UEqn.H"
            #include "pEqn.H"
        }

        turbulence->correct();

        if ( runTime.outputTime() )
        {
            ptot = p+0.5*magSqr(U);
        }

        systemPressureDrop(mesh,rho);

        runTime.write();

        Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
            << "  ClockTime = " << runTime.elapsedClockTime() << " s"
            << nl << endl;
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
