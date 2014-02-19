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



// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(structuralMode, 0);
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

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
    odeSubSteps_(1),

    modeShape_(dict_.subDict("modeShape"), BC),

    frequency_(readScalar(dict_.lookup("frequency"))),
    damping_(readScalar(dict_.lookup("damping"))),

    sweptVols_(mesh_.boundaryMesh()[patch_.index()].size()),

    motionEquation_(frequency_, damping_),
    odeSolver_(ODESolver::New("RK",motionEquation_))
{
    modeShape_.generate();
    calculateSweptVols();
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

scalar Foam::structuralMode::Q(const volScalarField& p) const
{
    return gSum(p.boundaryField()[patch_.index()]*sweptVols_);
}

scalar structuralMode::solveMotionEquation2
(
    const volScalarField& p
)
{
    const scalar q = Q(p);

    motionEquation_.RHS(q);

    scalarField dyStart(motionEquation_.nEqns());
    scalar notUsed = 0.0;

    motionEquation_.derivatives(notUsed, scalarField(odeData_), dyStart);

    scalar  t0 = mesh_.time().timeOutputValue();
    scalar  dT = mesh_.time().deltaTValue();
    scalar  t1 = t0+dT;

    scalar dtEst = (t1-t0)/10;

    scalarField y(odeData_);

    odeSolver_->solve(motionEquation_, t0, t1, y, 1e-5, dtEst);

    Info << "ODE solved with forcing Q ="
         << q << ". Coeffs = ("
         << odeData_[0] << " "
         << odeData_[1]
         << ") ODE dT = " << dtEst << endl;

    return y[0];
}

scalar structuralMode::solveMotionEquation
(
    const volScalarField& p
)
{
    // Solve the ODE using simple forward euler RK:
    // ddt2(a)+w^2 a = Q
    // Should include damping D so:
    // ddt2(a)+D*w*ddt(a)+w^2 a = Q

    scalar relaxation = 0.25;

    scalar dT = (mesh_.time().deltaTValue())/odeSubSteps_;

    scalar& aStar_ = odeData_[0];
    scalar& bStar_ = odeData_[1];

    scalar a = 0;
    scalar b = 0;

    // Calculate forcing (ODE RHS), as function of pressure
    const scalar q = Q(p);

    scalar omega = 2*Foam::constant::mathematical::pi*frequency_;

    for (int i=0; i<odeSubSteps_; i++)
    {
        b = dT*(q-pow(omega,2)*aStar_-2*damping_*omega*bStar_)+bStar_;
        a = dT*b+aStar_;

        aStar_ = a; // (1-relaxation)*a+relaxation*aStar_;
        bStar_ = b; //(1-relaxation)*b+relaxation*bStar_;
    }
    Info << "Unit work by mode    \""<< name_ <<"\" = " << q << endl;
    Info << "Coefficient for mode \""<< name_ <<"\" = " << a << endl;

    return a;
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

    os << mode.modeShape_ << endl;

    os.writeKeyword("frequency") << mode.frequency_
                                 << token::END_STATEMENT << nl;

    os  << decrIndent << token::END_BLOCK << nl;

    return os;
}

// ************************************************************************* //
