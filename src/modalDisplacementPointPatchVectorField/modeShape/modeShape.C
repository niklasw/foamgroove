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
//#include "fvCFD.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //


// * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * //


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

const scalarField Foam::modeShape::X()
{
    origin_ = dict_.lookup("origin");
    axis_ = dict_.lookup("axis");
    axis_ /= mag(axis_);
    scalarField modeCoordinate = axis_ & (points_ - origin_);
    return modeCoordinate;
}

void Foam::modeShape::genRigidMode()
{
    amplitude_ = dict_.lookup("amplitude");
    scalar scalingFactor_ = readScalar(dict_.lookup("scalingFactor"));
    displacement_ = vectorField(points_.size(), scalingFactor_*amplitude_);
}

void Foam::modeShape::genTrigonometricMode()
{
    waveLength_ = readScalar(dict_.lookup("waveLength"));
    amplitude_ = dict_.lookup("amplitude");
    scalingFactor_ = readScalar(dict_.lookup("scalingFactor"));

    scalar pi = constant::mathematical::pi;
    displacement_ = scalingFactor_*amplitude_
              * (1-cos((2*pi/waveLength_)*X()));
}

void Foam::modeShape::genPolynomialMode()
{
    vector amplitude_ = dict_.lookup("amplitude");
    List<scalar> coeffs;
    dict_.lookup("coeffs") >> coeffs;

    scalingFactor_ = readScalar(dict_.lookup("scalingFactor"));
    tmp<scalarField> dPtr(new scalarField(points_.size()));
    scalarField& D = dPtr();

    forAll(coeffs, i)
    {
        scalar C = coeffs[i];
        D += C*pow(X(),i);
    }
    displacement_ = amplitude_*D;
}

void Foam::modeShape::genInterpolatedMode()
{
    tmp<vectorField> tmpDisplacement(dict_.lookup("modeDisplacement"));
    scalingFactor_ = readScalar(dict_.lookup("scalingFactor"));
    displacement_ = scalingFactor_*tmpDisplacement();
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

    scalingFactor_(1.0),
    origin_(Foam::vector::zero),
    axis_(Foam::vector(0,0,1)),
    amplitude_(Foam::vector::zero),
    waveLength_(1.0),

    BC_(BC),
    points_(BC.patch().localPoints()),
    displacement_(points_.size(), vector::zero)
{}

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

void Foam::modeShape::generate()
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

    Info << "Mode max displacement = " << gMax(displacement_) << endl;
}

const Foam::vectorField& Foam::modeShape::displacement() const
{
    return displacement_;
}

const scalar& Foam::modeShape::scalingFactor() const
{
    return scalingFactor_;
}

scalar& Foam::modeShape::scalingFactor()
{
    return scalingFactor_;
}


// * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * * //

Foam::Ostream& Foam::operator<<(Ostream& os, const modeShape& shape)
{
    os.write("modeShape") << nl;
    os << token::BEGIN_BLOCK << incrIndent << nl;
    os.writeKeyword("modeType") << shape.modeTypeName_
                                << token::END_STATEMENT << nl;
    os.writeKeyword("ampliptude") << shape.amplitude_
                                  << token::END_STATEMENT << nl;
    os.writeKeyword("scalingFactor") << shape.scalingFactor()
                                     << token::END_STATEMENT << nl;
    os.writeKeyword("waveLength") << shape.waveLength_
                                  << token::END_STATEMENT << nl;
    os.writeKeyword("modeDisplacement") << shape.displacement()
                                        << token::END_STATEMENT << nl;
    os  << decrIndent << token::END_BLOCK << nl;

    return os;
}


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
