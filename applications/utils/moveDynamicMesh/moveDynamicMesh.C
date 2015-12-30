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
    moveDynamicMesh

Description
    Mesh motion and topological mesh changes utility.

\*---------------------------------------------------------------------------*/

#include "argList.H"
#include "fvCFD.H"
#include "Time.H"
#include "dynamicFvMesh.H"
#include "vtkSurfaceWriter.H"
#include "cyclicAMIPolyPatch.H"
#include "zeroGradientFvPatchFields.H"

using namespace Foam;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Dump patch + weights to vtk file
void writeWeights
(
    const scalarField& wghtSum,
    const primitivePatch& patch,
    const fileName& folder,
    const fileName& prefix,
    const word& timeName
)
{
    vtkSurfaceWriter writer;

    writer.write
    (
        folder,
        prefix + "_proc" + Foam::name(Pstream::myProcNo()) + "_" + timeName,
        patch.localPoints(),
        patch.localFaces(),
        "weightsSum",
        wghtSum,
        false
    );
}


void writeWeights(const polyMesh& mesh)
{
    const polyBoundaryMesh& pbm = mesh.boundaryMesh();

    const word tmName(mesh.time().timeName());

    forAll(pbm, patchI)
    {
        if (isA<cyclicAMIPolyPatch>(pbm[patchI]))
        {
            const cyclicAMIPolyPatch& cpp =
                refCast<const cyclicAMIPolyPatch>(pbm[patchI]);

            if (cpp.owner())
            {
                Info<< "Calculating AMI weights between owner patch: "
                    << cpp.name() << " and neighbour patch: "
                    << cpp.neighbPatch().name() << endl;

                const AMIPatchToPatchInterpolation& ami =
                    cpp.AMI();
                writeWeights
                (
                    ami.tgtWeightsSum(),
                    cpp.neighbPatch(),
                    "output",
                    "tgt",
                    tmName
                );
                writeWeights
                (
                    ami.srcWeightsSum(),
                    cpp,
                    "output",
                    "src",
                    tmName
                );
            }
        }
    }
}



int main(int argc, char *argv[])
{
    #include "addRegionOption.H"
    argList::addBoolOption
    (
        "checkMesh",
        "check mesh quality each step"
    );
    argList::addBoolOption
    (
        "checkAMI",
        "check AMI weights"
    );

    #include "setRootCase.H"
    #include "createTime.H"
    #include "createNamedDynamicFvMesh.H"

    const bool checkAMI  = args.optionFound("checkAMI");
    const bool checkMesh  = args.optionFound("checkMesh");

    if (checkAMI)
    {
        Info<< "Writing VTK files with weights of AMI patches." << nl << endl;
    }

    mesh.update();

    volScalarField surfaceSumMeshPhi
    (
        IOobject
        (
            "surfaceSumMeshPhi",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        fvc::surfaceSum(mesh.phi())
    );

    surfaceScalarField meshPhi
    (
        IOobject
        (
            "meshPhi",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mesh.phi()
    );

    volScalarField V
    (
        IOobject
        (
            "V",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensionedScalar("v",dimVolume,1.0)
    );

    volScalarField dVdt
    (
        IOobject
        (
            "dVdt",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensionedScalar("v",dimVolume,1.0)/dimensionedScalar("t",dimTime,1.0)
    );

    volScalarField constantScalarField
    (
        IOobject
        (
            "constantScalarField",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mesh.C().component(vector::X),
        zeroGradientFvPatchScalarField::typeName
    );


    while (runTime.loop())
    {
        Info<< "Time = " << runTime.timeName() << endl;

        mesh.update();

        V.internalField() = mesh.V();
        dVdt.internalField() = fvc::ddt(V);

        if (checkMesh)
        {
            mesh.checkMesh(true);
        }

        if (checkAMI)
        {
            writeWeights(mesh);
        }


        if (mesh.moving())
        {
            surfaceSumMeshPhi = fvc::surfaceSum(mesh.phi());
            meshPhi = mesh.phi();
            /*
             * Here seems to be a bug in OF fvMesh. mesh.Sf() is not
             * correctly updated on mesh changes?
            surfaceVectorField faceNormals
            (
                IOobject
                (
                    "surfaceNormals",
                    runTime.timeName(),
                    mesh,
                    IOobject::NO_READ,
                    IOobject::AUTO_WRITE
                ),
                mesh.Sf()
            );
            */
        }

        Info << "Min, max div(mesh.phi()) "
             << gMax(surfaceSumMeshPhi) << " " << gMin(surfaceSumMeshPhi) << endl;

        runTime.write();

        Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
            << "  ClockTime = " << runTime.elapsedClockTime() << " s"
            << nl << endl;
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
