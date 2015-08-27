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

\*---------------------------------------------------------------------------*/

#include "Time.H"
#include "meshTools.H"
#include "OFstream.H"
#include "volFields.H"
#include "proj.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::proj::proj
(
    const dictionary& dict,
    const gunTopoFvMesh& mesh
)
:
    projDict_(dict),
    mesh_(mesh),
    motionType_(projDict_.lookup("motion")),
    initialPosition_(readScalar(projDict_.lookup("initialPosition"))),
    initialVelocity_(readScalar(projDict_.lookup("initialVelocity"))),
    currentVelocity_(initialVelocity_),
    currentPosition_(initialPosition_),
    mass_(1.0),
    frictionCoeff_(0.0),
    patchNames_(0),
    patchLabels_(0)

{
    if (motionType_ == "linear")
    {
        exitVelocity_ = readScalar
        (
            projDict_.subDict("linearCoeffs").lookup("exitVelocity")
        );
    }
    else if (motionType_ == "dynamic")
    {
        const dictionary& d = projDict_.subDict("dynamicCoeffs");
        mass_ = readScalar(d.lookup("mass"));

        frictionCoeff_ = readScalar(d.lookup("frictionCoeff"));
        patchNames_.append(d.lookup("patches"));

        forAll(patchNames_, nameI)
        {
            patchLabels_.append(mesh_.boundary().findPatchID(patchNames_[nameI]));
        }
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::proj::~proj()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::proj::info() const
{
    Info << "\tBullet info: "
         << " velocity= " << currentVelocity() << "\t"
         << " position= " << currentPosition() << endl;

    return;
}

bool Foam::proj::update()
{
    Info << "Entering Foam::proj::update()" << endl;
    scalar dt = mass_*mesh_.time().deltaT().value();
    currentPosition_ += 0.5*dt*currentVelocity_; // FIXME
    updateVelocity();
    currentPosition_ += 0.5*dt*currentVelocity_; // FIXME

    return true;
}

void Foam::proj::updateVelocity()
{
    if (motionType_ == "dynamic")
    {
        scalar dVel = Foam::mag(pressureForce())/mass_*mesh_.time().deltaT().value();
        currentVelocity_ += dVel;
    }
    else if (motionType_ == "linear")
    {
        WarningIn("Foam::proj::updateVelocity()")
            << "motionType_ " << motionType_
            << " NOT IMPLEMENTED" << endl;
    }
}

Foam::scalar Foam::proj::currentPosition() const
{
    return currentPosition_;
}

Foam::scalar Foam::proj::currentVelocity() const
{
    if (motionType_ == "constant" )
    {
        return currentVelocity_;
    }
    else if (motionType_ == "linear")
    {
        return currentVelocity_;
    }
    return currentVelocity_;
}

Foam::vector Foam::proj::pressureForce()
{
    if (mesh_.foundObject<volScalarField>("p"))
    {
        const volScalarField& p = mesh_.lookupObject<volScalarField>("p");
        vector forceIntegral = vector::zero;
        forAll(patchLabels_, i)
        {
            label patchI = patchLabels_[i];
            const fvPatchScalarField& patchPressure = p.boundaryField()[patchI];
            forceIntegral += Foam::gSum
                (
                    patchPressure
                  * mesh_.Sf().boundaryField()[patchI]
                );
        }

        return (forceIntegral & mesh_.aimVector())*mesh_.aimVector();
    }
    else
    {
        FatalErrorIn("Foam::proj::pressureForce")
            << "Pressure field, p, not found" << exit(FatalError);
    }
    return vector::zero;
}
// ************************************************************************* //
