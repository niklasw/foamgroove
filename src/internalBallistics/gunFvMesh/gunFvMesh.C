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

#include "gunFvMesh.H"
#include "Time.H"
#include "addToRunTimeSelectionTable.H"
#include "meshTools.H"
#include "OFstream.H"
#include "pointFields.H"
#include "volFields.H"
#include "SortableList.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(gunFvMesh, 0);

    addToRunTimeSelectionTable
    (
        dynamicFvMesh,
        gunFvMesh,
        IOobject
    );
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::gunFvMesh::debugInfo()
{
    Info << "\tBullet info: "
         <<"time:" << time().value()
         << " curMotionVel_:" << curMotionVel_ << endl;
    Info << "\tBullet info: "
         << "Current volume of domain = " << gSum(V()) << endl;

    return;
}

const boundBox Foam::gunFvMesh::gunBounds(const List<label>& pointLabels) const
{
    List<point> motionPointList(pointLabels.size());

    forAll(pointLabels,i)
    {
        motionPointList[i] = points()[pointLabels[i]];
    }
    return  boundBox(motionPointList);
}

Foam::tmp<Foam::scalarField> Foam::gunFvMesh::motionMask() const
{
    tmp<scalarField> tmotionMask(new scalarField(points().size(),0.0));
    scalarField& motionMask = tmotionMask();

    boundBox barrelBB = gunBounds(gunPoints_);
    boundBox motionBB = gunBounds(motionPoints_);

    scalar expansionLength = motionBB.min().x() - barrelBB.min().x();

    forAll(gunPoints_, i)
    {
        label curPointI = gunPoints_[i];
        point curPoint = points()[curPointI];

        scalar boltDistance = curPoint.x()-barrelBB.min().x();

        motionMask[curPointI] = max(min(Foam::pow(boltDistance/expansionLength,0.8), 1.0),0.0);
    }

    return tmotionMask;
}

void Foam::gunFvMesh::gunMarkup()
{
    pointSet barrelPoints(*this,barrelPointSet_);
    pointSet motionPoints(*this,motionPointSet_);

    gunPoints_.append(barrelPoints.toc());
    motionPoints_.append(motionPoints.toc());
}

void Foam::gunFvMesh::linearizeU() const
{
    if
    (
        (! foundObject<volVectorField>("U"))
     || (! foundObject<volScalarField>("p"))
     || (! foundObject<volScalarField>("rho"))
    )
    {
        return;
    }
    if (time().value() <= time().deltaT().value())
    {
        Info << "WARNING: Foam::gunFvMesh::linearizeU() sets linear U in barrel" << endl;
        volVectorField& U = const_cast<volVectorField&>
                            (
                                lookupObject<volVectorField>("U")
                            );

        volScalarField& p = const_cast<volScalarField&>
                            (
                                lookupObject<volScalarField>("p")
                            );

        volScalarField& rho = const_cast<volScalarField&>
                            (
                                lookupObject<volScalarField>("rho")
                            );

        boundBox bb = gunBounds(gunPoints_);
        scalar bbMin = bb.min().x();
        scalar bbLength = bb.max().x()-bb.min().x();

        Info << "U is linearized in x > " << bb.min().x()
                               << " x < " << bb.max().x() << endl;

        forAll (cellCentres(), i)
        {

            point C = cellCentres()[i];
            if ( bb.contains(C) )
            {
                scalar fraction = (C.x()-bbMin)/bbLength;
                U[i] = initialVelocity_*fraction*aimVector_;
                p[i] -= 0.5*rho[i]*Foam::magSqr(U[i]);
            }
        }
        U.write();
        p.write();
    }
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Construct from components
Foam::gunFvMesh::gunFvMesh(const IOobject& io)
:
    dynamicFvMesh(io),
    motionDict_
    (
        IOdictionary
        (
            IOobject
            (
                "dynamicMeshDict",
                time().constant(),
                *this,
                IOobject::MUST_READ_IF_MODIFIED,
                IOobject::NO_WRITE,
                false
            )
        ).subDict(typeName + "Coeffs")
    ),
    aimVector_(motionDict_.lookup("aimVector")),
    boltPosition_(motionDict_.lookup("boltPosition")),
    barrelLength_(readScalar(motionDict_.lookup("barrelLength"))),
    barrelPointSet_(motionDict_.lookupOrDefault<word>("barrelPointSet","barrelPoints")),
    motionPointSet_(motionDict_.lookupOrDefault<word>("motionPointSet","motionPoints")),
    initialPosition_(readScalar(motionDict_.lookup("initialPosition"))),
    initialVelocity_(readScalar(motionDict_.lookup("initialVelocity"))),
    exitVelocity_(readScalar(motionDict_.lookup("exitVelocity"))),
    curMotionVel_(initialVelocity_),
    currentPosition_(initialPosition_),
    tolerance_(SMALL),
    gunPoints_(0),
    motionPoints_(0),
    nOldPoints_(0)
{
    //normalize aimVector
    if ( mag(aimVector_) <= SMALL )
    {
        FatalErrorIn("gunFvMesh::gunFvMesh: ")
            << "Aim vector too short."
            << abort(FatalError);
    }

    aimVector_ /= (mag(aimVector_)+SMALL);

    Info<< "Initial time:" << time().value()
        << " Initial curMotionVel_:" << curMotionVel_
        << endl;

    gunMarkup();

}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::gunFvMesh::~gunFvMesh()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::gunFvMesh::update()
{
    linearizeU();

    nOldPoints_ = points().size();

    pointField newPoints;

    curMotionVel_ = curMotionVel();

    debugInfo();

    vector pointsDisplacement =
        aimVector_ * curMotionVel()*time().deltaT().value();

    newPoints = points()
              + pointsDisplacement * motionMask();

    Info << "Executing mesh motion" << endl;

    movePoints(newPoints);

    return true;
}


Foam::scalar Foam::gunFvMesh::curMotionVel()
{
    curMotionVel_ = initialVelocity_;
    return curMotionVel_;
}

// ************************************************************************* //
