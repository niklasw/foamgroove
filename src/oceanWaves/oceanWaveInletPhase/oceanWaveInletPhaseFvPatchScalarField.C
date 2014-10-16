/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2006-2009 OpenCFD Ltd.
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

#include "oceanWaveInletPhaseFvPatchScalarField.H"
#include "volFields.H"
#include "surfaceFields.H"
#include "addToRunTimeSelectionTable.H"
#include "fvPatchFieldMapper.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //
Foam::tmp<Foam::scalarField> Foam::oceanWaveInletPhaseFvPatchScalarField::
surfacePosition()
{
    /* Note: omega = c*k */
    scalarField Z = (patch().Cf() & owf_.up()) - owf_.seaLevel();
    scalarField X = patch().Cf() & owf_.waveDirection();
    scalar t = db().time().timeOutputValue();

    tmp<scalarField> tInletSurface(new scalarField(this->size(),1.0));
    scalarField& inletSurface = tInletSurface();

    scalar h = owf_.elevation(t,0.0);

    forAll (Z, faceI)
    {
        scalar z = Z[faceI];
        if ( z > h )
        {
            inletSurface[faceI] = 0;
        }
    }
    return tInletSurface;
}

Foam::oceanWaveInletPhaseFvPatchScalarField::
oceanWaveInletPhaseFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchField<scalar>(p, iF),
    owf_(db())
{}

Foam::oceanWaveInletPhaseFvPatchScalarField::
oceanWaveInletPhaseFvPatchScalarField
(
    const oceanWaveInletPhaseFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchField<scalar>(ptf, p, iF, mapper),
    owf_(ptf.owf_)
{}

Foam::oceanWaveInletPhaseFvPatchScalarField::
oceanWaveInletPhaseFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchField<scalar>(p, iF, dict),
    owf_(db())
{
}

Foam::oceanWaveInletPhaseFvPatchScalarField::
oceanWaveInletPhaseFvPatchScalarField
(
    const oceanWaveInletPhaseFvPatchScalarField& ptf
)
:
    fixedValueFvPatchField<scalar>(ptf),
    owf_(ptf.owf_)
{}


Foam::oceanWaveInletPhaseFvPatchScalarField::
oceanWaveInletPhaseFvPatchScalarField
(
    const oceanWaveInletPhaseFvPatchScalarField& ptf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchField<scalar>(ptf, iF),
    owf_(ptf.owf_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::oceanWaveInletPhaseFvPatchScalarField::
updateCoeffs()
{
    if (updated())
    {
        return;
    }

    operator==(surfacePosition());

    fixedValueFvPatchField<scalar>::updateCoeffs();
}


void Foam::oceanWaveInletPhaseFvPatchScalarField::write
(
    Ostream& os
) const
{
    fvPatchField<scalar>::write(os);
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    makePatchTypeField
    (
        fvPatchScalarField,
        oceanWaveInletPhaseFvPatchScalarField
    );
}

// ************************************************************************* //
