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

Foam::vectorField Foam::oceanWaveInletVelocityFvPatchVectorField::
inletVelocity()
{
    /* Note: omega = c*k */
    vector gDirection = -g_/mag(g_);
    scalarField Z = (patch().Cf() & gDirection) - seaLevel_;
    scalarField X = patch().Cf() & waveDirection_;
    scalar t = db().time().timeOutputValue();

    vectorField inletVelocity(this->size(),vector(0,0,0));

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
        //scalar x = X[faceI];
        if ( z+seaLevel_ > h )
        {
            inletVelocity[faceI] = freeStreamVelocity_;
        }
        else
        {
            //scalar UX = waveVelocity_ * cos(k*x-omega*t)*cosh(k*z+k*waterDepth_);
            //scalar UZ = waveVelocity_ * sin(k*x-omega*t)*sinh(k*z+k*waterDepth_);
            scalar Ux, Uz;

            waveVelocities
            (
                waveLength_,
                waveHeight_,
                waterDepth_,
                g,
                t,
                0.0,
                z,
                Ux,
                Uz
            );

            inletVelocity[faceI] = freeStreamVelocity_ + waveDirection_*Ux + gDirection*Uz;
            //Info << "Ux Uz:\t" <<  Ux << "\t" << Uz << endl;
        }
    }
    return inletVelocity;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::oceanWaveInletVelocityFvPatchVectorField::oceanWaveInletVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchField<vector>(p, iF),
    seaLevel_(0.0),
    waveHeight_(0.1),
    waveLength_(1.0),
    waterDepth_(10.0),
    waveDirection_(vector(1,0,0)),
    waveVelocity_(0),
    g_(vector::zero)
{}


Foam::oceanWaveInletVelocityFvPatchVectorField::oceanWaveInletVelocityFvPatchVectorField
(
    const oceanWaveInletVelocityFvPatchVectorField& pivpvf,
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchField<vector>(pivpvf, p, iF, mapper),
    seaLevel_(pivpvf.seaLevel_),
    waveHeight_(pivpvf.waveHeight_),
    waveLength_(pivpvf.waveLength_),
    waterDepth_(pivpvf.waterDepth_),
    waveDirection_(pivpvf.waveDirection_),
    waveVelocity_(pivpvf.waveVelocity_),
    g_(pivpvf.g_)
{}


Foam::oceanWaveInletVelocityFvPatchVectorField::oceanWaveInletVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchField<vector>(p, iF),
    freeStreamVelocity_(dict.lookup("freeStreamVelocity"))
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

    seaLevel_ = oceanWavesDict.lookupOrDefault("seaLevel",scalar(0));
    waveHeight_ = oceanWavesDict.lookupOrDefault("waveHeight",scalar(1));
    waveLength_ = oceanWavesDict.lookupOrDefault("waveLength",scalar(20));
    waterDepth_ = oceanWavesDict.lookupOrDefault("waterDepth",scalar(10.0));
    waveDirection_ = oceanWavesDict.lookupOrDefault("waveDirection",vector(1,0,0));
    //g_ = oceanWavesDict.lookupOrDefault("g",vector(0,0,-9.81));

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
    waveVelocity_ = sqrt(mag(g_)*waveLength_/(2*pi)*tanh(2*pi*waterDepth_/waveLength_)); //Checked ok.
    waveDirection_ /= mag(waveDirection_);

    Info << "oceanWaveInletVelocityFvPatch:\n"
         << "\n - seaLevel     = " << seaLevel_
         << "\n - waveHeight= " << waveHeight_
         << "\n - waveLength   = " << waveLength_
         << "\n - waveDirection= " << waveDirection_
         << "\n - waveVelocity = " << waveVelocity_ << nl << endl;
}


Foam::oceanWaveInletVelocityFvPatchVectorField::oceanWaveInletVelocityFvPatchVectorField
(
    const oceanWaveInletVelocityFvPatchVectorField& pivpvf
)
:
    fixedValueFvPatchField<vector>(pivpvf),
    seaLevel_(pivpvf.seaLevel_),
    waveHeight_(pivpvf.waveHeight_),
    waveLength_(pivpvf.waveLength_),
    waterDepth_(pivpvf.waterDepth_),
    waveDirection_(pivpvf.waveDirection_),
    waveVelocity_(pivpvf.waveVelocity_),
    freeStreamVelocity_(pivpvf.freeStreamVelocity_),
    g_(pivpvf.g_)
{}


Foam::oceanWaveInletVelocityFvPatchVectorField::oceanWaveInletVelocityFvPatchVectorField
(
    const oceanWaveInletVelocityFvPatchVectorField& pivpvf,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchField<vector>(pivpvf, iF),
    seaLevel_(pivpvf.seaLevel_),
    waveHeight_(pivpvf.waveHeight_),
    waveLength_(pivpvf.waveLength_),
    waterDepth_(pivpvf.waterDepth_),
    waveDirection_(pivpvf.waveDirection_),
    waveVelocity_(pivpvf.waveVelocity_),
    freeStreamVelocity_(pivpvf.freeStreamVelocity_),
    g_(pivpvf.g_)
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
    os.writeKeyword("freeStreamVelocity") << freeStreamVelocity_ << token::END_STATEMENT << nl;
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
