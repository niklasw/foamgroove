/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2012 OpenFOAM Foundation
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

#include "structuralMode.H"
#include "polyMesh.H"
#include "volFields.H"
#include "surfaceFields.H"
#include "fvMatrices.H"
#include "syncTools.H"
#include "faceSet.H"
#include "geometricOneField.H"
#include "pointPatch.H"
#include "pointBoundaryMesh.H"
#include "pointMesh.H"
#include "OFstream.H"

#include "myODE.H"


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(structuralMode, 0);
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::structuralMode::scale()
{
    Info << "Scaling mode "<<name_ << " with factor "
         << scalingFactor_ << endl; 
    modeDisplacement_ *= scalingFactor_;
}

void Foam::structuralMode::distributeParModeDisplacement()
{
    Info << "Distributing mode \"" <<name()<<"\" to processors" << nl << endl;

    vectorField localModeDisplacement(patch_.size(), vector::zero);

    forAll (patch_.meshPoints(), i)
    {
        label globalPatchIndex = BC_.localToGlobalPatchIndex(i);
        localModeDisplacement[i] = modeDisplacement_[globalPatchIndex];
    }

    modeDisplacement_.resize(patch_.size());
    modeDisplacement_ = localModeDisplacement;
}

void structuralMode::calculateSweptVols()
{
    Info << "Calculating volumes swept by mode \""<< name() << "\"\n" << endl;
    label patchIndex = patch_.index();

    const pointField& patchPoints = patch_.localPoints();

    pointField tmpPatchPoints(patchPoints+modeDisplacement());

    const faceList& pFaces = mesh_.boundaryMesh()[patchIndex].localFaces();

    forAll(pFaces, faceI)
    {
        const face& f = pFaces[faceI];

        sweptVols_[faceI] = f.sweptVol
                            (
                                patch_.localPoints(),
                                tmpPatchPoints
                            );
    }
}

scalar structuralMode::calculateCoeff
(
    const volScalarField& p
)
{
    scalar dT = (mesh_.time().deltaT().value())/odeSubSteps_;

    scalar& aStar_ = odeData_[0];
    scalar& bStar_ = odeData_[1];

    scalar a = 0;

    const scalar q = Q(p);

    scalar omega = 2*Foam::constant::mathematical::pi*frequency_;

    for (int i=0; i<odeSubSteps_; i++)
    {
        scalar b = dT*(q-pow(omega,2)*aStar_)+bStar_;
               a = dT*b+aStar_;

        aStar_ = a;
        bStar_ = b;
    }
    Info << "Unit work by mode    \""<< name_ <<"\" = " << q << endl;
    Info << "Coefficient for mode \""<< name_ <<"\" = " << a << endl;

    return a;
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::structuralMode::structuralMode
(
    const modalDisplacementPointPatchField& BC,
    Istream& is
)
:
    BC_(BC),
    patch_(BC.patch()),
    mesh_(BC.dimensionedInternalField().mesh()()),
    name_(is),
    dict_(is),
    odeData_(2, 0.0),
    odeSubSteps_(50),

    generateMode_(dict_.lookupOrDefault("generateMode",false)),
    origin_(vector::zero),
    axis_(vector(1,0,0)),
    waveLength_(1.0),
    amplitude_(vector::zero),

    frequency_(readScalar(dict_.lookup("frequency"))),
    scalingFactor_(readScalar(dict_.lookup("scalingFactor"))),
    modeDisplacement_(dict_.lookup("modeDisplacement")),
    sweptVols_(mesh_.boundaryMesh()[patch_.index()].size())

{
    bool parallel = Pstream::parRun();

    axis_ = axis_/mag(axis_);

    if (modeDisplacement_.size()==0)
    {
        WarningIn("Foam::structuralMode::structuralMode")
            << "modeDisplacement list with zero elements!" << nl
            << "Resizing with vector::zero elements.\n" << endl;

        modeDisplacement_.resize(patch_.size(),vector::zero);
    }

    if (generateMode_)
    {
        Info << "Generating tronometric mode " << this->name()
             <<" for patch " << patch_.name() << nl << endl;

        const dictionary& genDict = dict_.subDict("generatedMode");

        origin_ = genDict.lookup("origin");
        axis_ = genDict.lookup("axis");
        waveLength_ = readScalar(genDict.lookup("waveLength"));
        amplitude_ = genDict.lookup("amplitude");

        modeDisplacement_.resize(patch_.size(),vector::zero);
        trigonometricMode();
    }

    else
    {
        if (parallel)
        {
            distributeParModeDisplacement();
        }
        else
        {
            if (modeDisplacement_.size() != patch_.size())
            {
                WarningIn("Foam::structuralMode::structuralMode")
                    << "modeDisplacement list size (" << modeDisplacement_.size()
                    << ") does not match size of patch (" << patch_.size() << nl
                    << ") This is normal during decomposePar/reconstructPar!"
                    << nl << endl;
            }
        }
    }
    scale();
    calculateSweptVols();
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::structuralMode::trigonometricMode()
{
    scalar pi = constant::mathematical::pi;
    scalarField modeCoordinate = axis_ & (patch_.localPoints() - origin_);
    modeDisplacement_ = amplitude_
              * (1-cos((2*pi/waveLength_)*modeCoordinate));
}

void Foam::structuralMode::scale(const scalar reScale)
{
    scalingFactor_ = reScale;
    scale();
}

scalar Foam::structuralMode::Q(const volScalarField& p) const
{
    return gSum(p.boundaryField()[patch_.index()]*sweptVols_);
}

void Foam::structuralMode::write() const
{
     Info << "Writing mode to temporary file structuralMode_"
          <<  name() << endl;
     OFstream of("structuralMode_"+name());
     of << *this << endl;
     of << "\nCorresponding patch points" << endl;
     of << patch_.localPoints();
}

Foam::Ostream& Foam::operator<<(Ostream& os, const structuralMode& mode)
{
    os.write(mode.name_) << nl;

    os  << token::BEGIN_BLOCK << incrIndent << nl;

    os.writeKeyword("generateMode") << "no" << token::END_STATEMENT << nl;

    os.writeKeyword("generatedMode") << nl;

    os.indent();

    os << token::BEGIN_BLOCK << incrIndent << nl;

    os.writeKeyword("origin") << mode.origin_
                              << token::END_STATEMENT << nl;

    os.writeKeyword("axis") << mode.axis_ << token::END_STATEMENT << nl;

    os.writeKeyword("waveLength") << mode.waveLength_
                                  << token::END_STATEMENT << nl;

    os  << token::END_BLOCK << decrIndent << nl;

    os.writeKeyword("frequency") << mode.frequency_
                                 << token::END_STATEMENT << nl;

    os.writeKeyword("scalingFactor") << mode.scalingFactor_
                                     << token::END_STATEMENT << nl;

    os.writeKeyword("modeDisplacement") << mode.modeDisplacement_
                                        << token::END_STATEMENT << nl;

    os  << decrIndent << token::END_BLOCK << nl;

    return os;
}

// ************************************************************************* //
