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
    testModesDisplacement

Description
    Utility for preparation of a modal analysis FSI case.

    Usage
    =====
    A case, consisting of the solid side of the "modal" boundary is needed.

    testModesDisplacement -rho <scalar> [-writeDisplacements]

    (1) Activate one (1) mode in constant/structuralModes/modeData_{patchName}
        Make sure the scale is set to 1.0 for the mode.

    (2) In the 0/pointDisplacement field, set "writeDebugField yes;" to disable
        the FSI functions and dependence on flow fields.

    (3) Run the application, and define the solid density with argument -rho

    The resulting Scale factor is written to stdout, and has to manually be
    inserted for the mode.

\*---------------------------------------------------------------------------*/

#include "argList.H"
#include "Time.H"
#include "dynamicFvMesh.H"
#include "cyclicAMIPolyPatch.H"
#include "pointFields.H"
#include "volFields.H"
#include "fvc.H"
#include "fvMotionSolverCore.H"
#include "modalDisplacementPointPatchVectorField.H"

using namespace Foam;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    #include "setRootCase.H"
    #include "createTime.H"
    #include "createNamedDynamicFvMesh.H"

    Info<< "Time = " << runTime.timeName() << endl;

    const pointVectorField& pointDisplacement
        = mesh.lookupObject<pointVectorField>("pointDisplacement");

    forAll (pointDisplacement.boundaryField(), patchI)
    {
        const pointPatchField<vector>& curPatch =
            pointDisplacement.boundaryField()[patchI];

        if ( isA<modalDisplacementPointPatchField>(curPatch) )
        {
            Info << "Modal displacement patch: "
                 << mesh.boundary()[patchI].name() << endl;

            //- Here is a not very nice operations of casting (in the
            //  words of Prabaker, Shantaram) seems necessary to reach
            //  down through layers of patchFields in order to access
            //  the debugMode() member.

            const pointPatchField<vector>* Pptr = &curPatch;

            modalDisplacementPointPatchField* BC =
            dynamic_cast<modalDisplacementPointPatchField* >
            (
                const_cast<pointPatchField<vector>* >(Pptr)
            );

            // Set debugMode for the modalDisplacementPointPatchField,
            // since debugMode prevents normal fluid interaction but sets
            // the explicit displacements directly to the patch

            BC->debugMode() = true;
        }
    }

    //- Perform the mesh motion, in order to distribute the
    //  patch mode displacement in the solid.
    mesh.update();

    runTime++;

    pointDisplacement.write();

    mesh.write();

    Info<< "End\n" << endl;

    return 0;
}

// ************************************************************************* //
