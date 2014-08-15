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

#include "bulletDisplacementPointPatchVectorField.H"
#include "pointPatchFields.H"
#include "addToRunTimeSelectionTable.H"
#include "Time.H"
#include "polyMesh.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

bulletDisplacementPointPatchVectorField::
bulletDisplacementPointPatchVectorField
(
    const pointPatch& p,
    const DimensionedField<vector, pointMesh>& iF
)
:
    fixedValuePointPatchField<vector>(p, iF),
    aimVector_(vector(1,0,0)),
    boltPosition_(vector::zero),
    initialPosition_(0.0),
    currentPosition_(initialPosition_),
    barrelLength_(1.0),
    v0_(0.0),
    v1_(0.0),
    currentVelocity_(0.0)
{}


bulletDisplacementPointPatchVectorField::
bulletDisplacementPointPatchVectorField
(
    const pointPatch& p,
    const DimensionedField<vector, pointMesh>& iF,
    const dictionary& dict
)
:
    fixedValuePointPatchField<vector>(p, iF, dict),

    aimVector_(dict.lookup("aimVector")),
    boltPosition_(dict.lookup("boltPosition")),
    initialPosition_(readScalar(dict.lookup("initialPosition"))),
    barrelLength_(readScalar(dict.lookup("barrelLength"))),
    v0_(readScalar(dict.lookup("v0"))),
    v1_(readScalar(dict.lookup("v1"))),
    currentVelocity_(0.0)

{
    if (!dict.found("value"))
    {
        updateCoeffs();
    }
}


bulletDisplacementPointPatchVectorField::
bulletDisplacementPointPatchVectorField
(
    const bulletDisplacementPointPatchVectorField& ptf,
    const pointPatch& p,
    const DimensionedField<vector, pointMesh>& iF,
    const pointPatchFieldMapper& mapper
)
:
    fixedValuePointPatchField<vector>(ptf, p, iF, mapper),
    aimVector_(ptf.aimVector_),
    boltPosition_(ptf.boltPosition_),
    initialPosition_(ptf.initialPosition_),
    currentPosition_(ptf.currentPosition_),
    barrelLength_(ptf.barrelLength_),
    v0_(ptf.v0_),
    v1_(ptf.v1_),
    currentVelocity_(ptf.currentVelocity_)
{}


bulletDisplacementPointPatchVectorField::
bulletDisplacementPointPatchVectorField
(
    const bulletDisplacementPointPatchVectorField& ptf,
    const DimensionedField<vector, pointMesh>& iF
)
:
    fixedValuePointPatchField<vector>(ptf, iF),
    aimVector_(ptf.aimVector_),
    boltPosition_(ptf.boltPosition_),
    initialPosition_(ptf.initialPosition_),
    currentPosition_(ptf.currentPosition_),
    barrelLength_(ptf.barrelLength_),
    v0_(ptf.v0_),
    v1_(ptf.v1_),
    currentVelocity_(ptf.currentVelocity_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void bulletDisplacementPointPatchVectorField::updateCoeffs()
{
    if (this->updated())
    {
        return;
    }

    // Assume constant dv/dx between v0 and v1

    const polyMesh& mesh = this->dimensionedInternalField().mesh()();

    scalar accelerationLength = barrelLength_-initialPosition_;
    scalar dvdx = (v1_ - v0_)/accelerationLength;
    currentVelocity_ = min((currentPosition_ - initialPosition_)*dvdx, v1_);
    currentPosition_ += mesh.time().deltaTValue() * currentVelocity_;

    Field<vector>::operator=(boltPosition_+aimVector_*currentPosition_);

    fixedValuePointPatchField<vector>::updateCoeffs();
}


void bulletDisplacementPointPatchVectorField::write(Ostream& os) const
{
    pointPatchField<vector>::write(os);
    os.writeKeyword("aimVector")
        << aimVector_ << token::END_STATEMENT << nl;
    os.writeKeyword("boltPosition")
        << boltPosition_ << token::END_STATEMENT << nl;
    os.writeKeyword("initialPosition")
        << currentPosition_ << token::END_STATEMENT << nl;
    os.writeKeyword("barrelLength")
        << barrelLength_ << token::END_STATEMENT << nl;
    os.writeKeyword("v0")
        << v0_ << token::END_STATEMENT << nl;
    os.writeKeyword("v1")
        << v1_ << token::END_STATEMENT << nl;
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePointPatchTypeField
(
    pointPatchVectorField,
    bulletDisplacementPointPatchVectorField
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
