/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2013 OpenFOAM Foundation
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

#include "modalDisplacementPointPatchVectorField.H"
#include "modeShape.H"
#include "fvCFD.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //


// * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * //


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

const scalarField Foam::modeShape::coordinateAxis()
{
    Foam::vector origin_ = dict_.lookup("origin");
    Foam::vector axis_ = dict_.lookup("axis");
    axis_ /= mag(axis_);
    scalarField modeCoordinate = axis_ & (points_ - origin_);
    return modeCoordinate;
}

void Foam::modeShape::genRigidMode()
{
    vector amplitude_ = dict_.lookup("amplitude");
    scalar scale = readScalar(dict_.lookup("scalingFactor"));
    displacement_ = vectorField(points_.size(), scale*amplitude_);
}

void Foam::modeShape::genTrigonometricMode()
{
    scalar waveLength_ = readScalar(dict_.lookup("waveLength"));
    vector amplitude_ = dict_.lookup("amplitude");
    scalar scale = readScalar(dict_.lookup("scalingFactor"));

    scalar pi = constant::mathematical::pi;
    displacement_ = scale*amplitude_
              * (1-cos((2*pi/waveLength_)*coordinateAxis()));
}

void Foam::modeShape::genPolynomialMode()
{
    //vector amplitude_ = dict_.lookup("amplitude");
    //vector coeffs = dict_.lookup("polyCoeffs");
    //scalar scale = dict_.lookup("scalingFactor");
    //displacement_ = ...
}

void Foam::modeShape::genInterpolatedMode()
{
    tmp<vectorField> tmpDisplacement(dict_.lookup("modeDisplacement"));
    scalar scale = readScalar(dict_.lookup("scalingFactor"));
    displacement_ = scale*tmpDisplacement();
}

void Foam::modeShape::distributeParModeDisplacement()
{
    if (Pstream::parRun())
    {
        Info << "Distributing mode to processors" << nl << endl;

        vectorField localModeDisplacement(BC_.patch().size(), vector::zero);

        forAll (BC_.patch().meshPoints(), i)
        {
            label globalPatchIndex = BC_.localToGlobalPatchIndex(i);
            localModeDisplacement[i] = displacement_[globalPatchIndex];
        }

        displacement_.resize(BC_.patch().size());
        displacement_ = localModeDisplacement;
    }
}


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::modeShape::modeShape
(
    const dictionary& dict,
    const modalDisplacementPointPatchField& BC
)
:
    dict_(dict),
    modeTypeName_(dict_.lookup("modeType")),
    BC_(BC),
    points_(BC.patch().localPoints()),
    displacement_(points_.size(), vector::zero)
{
    if ( modeTypeName_ == "trigonometric" )
    {
        genTrigonometricMode();
    }
    else if ( modeTypeName_ == "polynomial" )
    {
        genPolynomialMode();
    }
    else if ( modeTypeName_ == "rigid" )
    {
        genRigidMode();
    }
    else if ( modeTypeName_ == "interpolated" )
    {
        genInterpolatedMode();
        distributeParModeDisplacement();
    }
}

// * * * * * * * * * * * * * * * * Selectors * * * * * * * * * * * * * * * * //

/*
Foam::autoPtr<Foam::modeShape>
Foam::modeShape::New()
{
    return autoPtr<modeShape>(new modeShape);
}
*/

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //


Foam::modeShape::~modeShape()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

const Foam::vectorField& Foam::modeShape::displacement() const
{
    return displacement_;
}

// * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * * //

void Foam::modeShape::operator=(const modeShape& rhs)
{
    // Check for assignment to self
    if (this == &rhs)
    {
        FatalErrorIn("Foam::modeShape::operator=(const Foam::modeShape&)")
            << "Attempted assignment to self"
            << abort(FatalError);
    }
}

// * * * * * * * * * * * * * * Friend Functions  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * Friend Operators * * * * * * * * * * * * * * //


// ************************************************************************* //
