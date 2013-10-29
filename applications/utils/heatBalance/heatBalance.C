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
    rhoSimpleFoam

Description
    Calculate heat and mass balance between two patches, e.g. inlet and outlet

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "rhoThermo.H"
#include "RASModel.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    timeSelector::addOptions();
    argList::validArgs.append("inletPatch");
    argList::validArgs.append("outletPatch");
    #include "setRootCase.H"
    #include "createTime.H"
    #include "createMesh.H"
    instantList timeDirs = timeSelector::select0(runTime, args);


    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    const word inletPatch = args[1];
    const word outletPatch = args[2];
    List<word> patchList(2);
    patchList[0] = args[1];
    patchList[1] = args[2];

    forAll (timeDirs, timeI)
    {
        runTime.setTime(timeDirs[timeI],timeI);
        Info<< "Time = " << runTime.timeName() << nl << endl;

        mesh.readUpdate();

        #include "createFields.H"

        #include "compressibleCreatePhi.H"

        {
            label patchI = mesh.boundaryMesh().findPatchID(patchList[0]);

            scalar heatFlux0 = gSum( phi.boundaryField()[patchI] * h.boundaryField()[patchI] );
            scalar massFlux0 = gSum( phi.boundaryField()[patchI]);

            patchI = mesh.boundaryMesh().findPatchID(patchList[1]);

            scalar heatFlux1 = gSum( phi.boundaryField()[patchI] * h.boundaryField()[patchI] );
            scalar massFlux1 = gSum( phi.boundaryField()[patchI]);

            Info<< "    Heat flux through patch 0"
                << '[' << patchList[0] << ']' << " = "
                << heatFlux0 << ", Mass flux = " << massFlux0 << endl;

            Info<< "    Heat flux through patch 1"
                << '[' << patchList[1] << ']' << " = "
                << heatFlux1 << endl;

            Info<< "    Heat balance = " << heatFlux0 +  heatFlux1 << endl;
            Info<< "    Mass balance = " << massFlux0 + massFlux1 << endl;
        }

        Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
            << "  ClockTime = " << runTime.elapsedClockTime() << " s"
            << nl << endl;
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
