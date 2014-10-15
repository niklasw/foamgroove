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
#include "mathematicalConstants.H"
#include "uniformDimensionedFields.H"
#include "oceanWaveFunctions.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //
Foam::tmp<Foam::scalarField> Foam::oceanWaveInletPhaseFvPatchScalarField::
surfacePosition()
{
    /* Note: omega = c*k */
    scalarField Z = patch().Cf() & (-g_/mag(g_));
    scalarField X = patch().Cf() & waveDirection_;
    scalar t = db().time().timeOutputValue();

    tmp<scalarField> inletSurfaceTmp(new scalarField(this->size(),1.0));
    scalarField& inletSurface = inletSurfaceTmp();

    scalar g = mag(g_);

    scalar h =
        elevation
        (
            waveLength_,
            waveHeight_,
            waterDepth_,
            g,
            t,
            0.0
        );

    forAll (Z, faceI)
    {
        scalar z = Z[faceI];
        if ( z > h )
        {
            inletSurface[faceI] = 0;
        }
    }
    return inletSurfaceTmp;
}

Foam::oceanWaveInletPhaseFvPatchScalarField::
oceanWaveInletPhaseFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchField<scalar>(p, iF),
    seaLevel_(0.0),
    waveHeight_(0.1),
    waveLength_(1.0),
    waterDepth_(10.0),
    waveVelocity_(0),
    waveDirection_(vector(1,0,0)),
    g_(vector::zero)
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
    seaLevel_(ptf.seaLevel_),
    waveHeight_(ptf.waveHeight_),
    waveLength_(ptf.waveLength_),
    waterDepth_(ptf.waterDepth_),
    waveVelocity_(ptf.waveVelocity_),
    waveDirection_(ptf.waveDirection_),
    g_(ptf.g_)
{}

Foam::oceanWaveInletPhaseFvPatchScalarField::
oceanWaveInletPhaseFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchField<scalar>(p, iF, dict)
{
    IOdictionary oceanWavesDict
    (
        Foam::IOobject
        (
            "oceanWavesDict",
            db().time().caseConstant(),
            db(),
            IOobject::MUST_READ
        )
    );

    oceanWaveFunctions owf(this->mesh());

    seaLevel_ = oceanWavesDict.lookupOrDefault("seaLevel",scalar(0));
    waveHeight_ = oceanWavesDict.lookupOrDefault("waveHeight",scalar(1));
    waveLength_ = oceanWavesDict.lookupOrDefault("waveLength",scalar(20));
    waterDepth_ = oceanWavesDict.lookupOrDefault("waterDepth",scalar(10.0));
    waveDirection_ = oceanWavesDict.lookupOrDefault("waveDirection",vector(1,0,0));
    //g_ = g.lookupOrDefault("value",vector(0,0,-9.81));
    Info<< "\nReading g" << endl;
    uniformDimensionedVectorField g
    (
        IOobject
        (
            "g",
            db().time().caseConstant(),
            db(),
            IOobject::MUST_READ
        )
    );

    g_ = g.value();

    scalar pi = constant::mathematical::pi;
    waveVelocity_ = sqrt(mag(g_)*waveLength_/(2*pi)*tanh(2*pi*waterDepth_/waveLength_));
    waveDirection_ /= (mag(waveDirection_)+SMALL);

    Info << "oceanWaveInletPhaseFvPatch:\n"
         << "\n - seaLevel     = " << seaLevel_
         << "\n - waveHeight= " << waveHeight_
         << "\n - waveLength   = " << waveLength_
         << "\n - waveDirection= " << waveDirection_
         << "\n - waveVelocity = " << waveVelocity_ << nl << endl;
}

Foam::oceanWaveInletPhaseFvPatchScalarField::
oceanWaveInletPhaseFvPatchScalarField
(
    const oceanWaveInletPhaseFvPatchScalarField& ptf
)
:
    fixedValueFvPatchField<scalar>(ptf),
    seaLevel_(ptf.seaLevel_),
    waveHeight_(ptf.waveHeight_),
    waveLength_(ptf.waveLength_),
    waterDepth_(ptf.waterDepth_),
    waveVelocity_(ptf.waveVelocity_),
    waveDirection_(ptf.waveDirection_),
    g_(ptf.g_)
{}


Foam::oceanWaveInletPhaseFvPatchScalarField::
oceanWaveInletPhaseFvPatchScalarField
(
    const oceanWaveInletPhaseFvPatchScalarField& ptf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchField<scalar>(ptf, iF),
    seaLevel_(ptf.seaLevel_),
    waveHeight_(ptf.waveHeight_),
    waveLength_(ptf.waveLength_),
    waterDepth_(ptf.waterDepth_),
    waveVelocity_(ptf.waveVelocity_),
    waveDirection_(ptf.waveDirection_),
    g_(ptf.g_)

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
