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

#include "freeSurfaceWaveOutletFvPatchScalarField.H"
#include "addToRunTimeSelectionTable.H"
#include "fvPatchFieldMapper.H"
#include "fixedValueFvPatchField.H"
#include "volFields.H"
#include "uniformDimensionedFields.H"


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::freeSurfaceWaveOutletFvPatchScalarField::
freeSurfaceWaveOutletFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedGradientFvPatchScalarField(p, iF)
{}


Foam::freeSurfaceWaveOutletFvPatchScalarField::
freeSurfaceWaveOutletFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    fixedGradientFvPatchScalarField(p, iF)
{
    fvPatchField<scalar>::operator=(patchInternalField());
    gradient() = 0.0;
}


Foam::freeSurfaceWaveOutletFvPatchScalarField::
freeSurfaceWaveOutletFvPatchScalarField
(
    const freeSurfaceWaveOutletFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedGradientFvPatchScalarField(ptf, p, iF, mapper)
{}


Foam::freeSurfaceWaveOutletFvPatchScalarField::
freeSurfaceWaveOutletFvPatchScalarField
(
    const freeSurfaceWaveOutletFvPatchScalarField& ptf
)
:
    fixedGradientFvPatchScalarField(ptf)
{}


Foam::freeSurfaceWaveOutletFvPatchScalarField::
freeSurfaceWaveOutletFvPatchScalarField
(
    const freeSurfaceWaveOutletFvPatchScalarField& ptf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedGradientFvPatchScalarField(ptf, iF)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::freeSurfaceWaveOutletFvPatchScalarField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    const label& thisPatchI = this->patch().index();

    const uniformDimensionedVectorField& g =
        db().lookupObject<uniformDimensionedVectorField>("g");

    const scalarField& internalField = this->patchInternalField()();

    const word& fieldName = this->dimensionedInternalField().name();

    const volScalarField& alpha1 =
        db().lookupObject<volScalarField>(fieldName);

    volVectorField gradAlpha
    (
        IOobject
        (
            "gradAlpha",
            this->db().time().timeName(),
            this->db(),
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        fvc::grad(alpha1),
        calculatedFvPatchField<vector>::typeName
    );

    //surfaceVectorField gradAlphaf(fvc::interpolate(gradAlpha));
    const fvPatchField<vector>& patchGradAlpha = gradAlpha.boundaryField()[thisPatchI];

    scalarField patchNormalGradAlpha = patch().nf() & patchGradAlpha; 

    Info << "Grad alpha " << patchGradAlpha << " "<< patch().nf()().size() << endl;

    gradient() = patch().nf() & patchGradAlpha;

    fixedGradientFvPatchScalarField::updateCoeffs();
}


void Foam::freeSurfaceWaveOutletFvPatchScalarField::write(Ostream& os) const
{
    fixedGradientFvPatchScalarField::write(os);
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    makePatchTypeField
    (
        fvPatchScalarField,
        freeSurfaceWaveOutletFvPatchScalarField
    );
}


// ************************************************************************* //
