/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 1991-2009 OpenCFD Ltd.
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

\*---------------------------------------------------------------------------*/

#include "oceanWaveInletVelocityFvPatchVectorField.H"
#include "volFields.H"
#include "surfaceFields.H"
#include "addToRunTimeSelectionTable.H"
#include "fvPatchFieldMapper.H"
#include "mathematicalConstants.H"
#include "uniformDimensionedFields.H"
#include "oceanWaveFunctions.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

Foam::tmp<Foam::vectorField> Foam::oceanWaveInletVelocityFvPatchVectorField::
inletVelocity()
{
    /* Note: omega = c*k */
    scalarField Z = (patch().Cf() & owf_.up()) - owf_.seaLevel();
    scalarField X = patch().Cf() & owf_.waveDirection();
    scalar t = db().time().timeOutputValue();

    tmp<vectorField> tInletVelocity(new vectorField(this->size(),vector(0,0,0)));
    vectorField& inletVelocity = tInletVelocity();

    scalar h = owf_.elevation(t,0.0);

    forAll (Z, faceI)
    {
        scalar z = Z[faceI];
        scalar x = 0.0; // X[faceI];
        if ( z > h )
        {
            inletVelocity[faceI] = owf_.freeStreamVelocity();
        }
        else
        {
            inletVelocity[faceI] = owf_.waveVelocities(x,z,t);
        }
    }
    return tInletVelocity;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::oceanWaveInletVelocityFvPatchVectorField::oceanWaveInletVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchField<vector>(p, iF),
    owf_(db())
{}


Foam::oceanWaveInletVelocityFvPatchVectorField::oceanWaveInletVelocityFvPatchVectorField
(
    const oceanWaveInletVelocityFvPatchVectorField& pvf,
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchField<vector>(pvf, p, iF, mapper),
    owf_(pvf.owf_)
{}


Foam::oceanWaveInletVelocityFvPatchVectorField::oceanWaveInletVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchField<vector>(p, iF),
    owf_(db())
{}


Foam::oceanWaveInletVelocityFvPatchVectorField::oceanWaveInletVelocityFvPatchVectorField
(
    const oceanWaveInletVelocityFvPatchVectorField& pvf
)
:
    fixedValueFvPatchField<vector>(pvf),
    owf_(pvf.owf_)
{}


Foam::oceanWaveInletVelocityFvPatchVectorField::oceanWaveInletVelocityFvPatchVectorField
(
    const oceanWaveInletVelocityFvPatchVectorField& pvf,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchField<vector>(pvf, iF),
    owf_(pvf.owf_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::oceanWaveInletVelocityFvPatchVectorField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    vectorField::operator=(inletVelocity());

    fixedValueFvPatchVectorField::updateCoeffs();
}


void Foam::oceanWaveInletVelocityFvPatchVectorField::write(Ostream& os) const
{
    fvPatchVectorField::write(os);
    //os.writeKeyword("freeStreamVelocity") << freeStreamVelocity_ << token::END_STATEMENT << nl;
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    makePatchTypeField
    (
        fvPatchVectorField,
        oceanWaveInletVelocityFvPatchVectorField
    );
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// ************************************************************************* //
