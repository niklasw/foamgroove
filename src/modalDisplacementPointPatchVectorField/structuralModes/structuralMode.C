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
    odeData_(structuralMode::nOdeData_, 0.0),
    odeSubSteps_(10),

    modeShape_(dict_.subDict("modeShape"), BC),

    frequency_(readScalar(dict_.lookup("frequency"))),
    damping_(readScalar(dict_.lookup("damping"))),

    sweptVols_(mesh_.boundaryMesh()[patch_.index()].size())

    //motionEquation_(frequency_, damping_),
    //odeSolver_(ODESolver::New("RK",motionEquation_))
{
    modeShape_.generate();
    calculateSweptVols();
    Info << "Added structuralMode " << name() << endl;
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

scalar Foam::structuralMode::calcQ(const volScalarField& p) const
{
    return gSum(p.boundaryField()[patch_.index()]*sweptVols_);
}

/*
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
*/

scalar structuralMode::solveMotionEquation
(
    const volScalarField& p,
    const word& odeSolver
)
{
    scalar q = 0;
    if ( odeSolver == "simple") q = simpleSolve(p);
    else if (odeSolver == "Newmark") q = NewmarkSolve(p);
    else if (odeSolver == "forcedMotion") q = forcedMotion();
    else
    {
        FatalErrorIn("structuralMode::solveMotionEquation")
            << "Wrong solver selection: Select any of simple,"
            << " Newmark or forcedMotion"
            <<  exit(FatalError);
    }

    return q;
}

scalar structuralMode::simpleSolve
(
    const volScalarField& p
)
{
    // Solve the ODE using simple forward euler:
    // ddt2(a)+D*w*ddt(a)+w^2 a = Q

    // scalar relax = 0.75;

    scalar dT = (mesh_.time().deltaTValue())/odeSubSteps_;

    scalar& a_0  = odeData_[0];
    scalar& b_0  = odeData_[1];
    scalar& Q_0  = odeData_[2];
    scalar& Q_00 = odeData_[3];

    scalar a = 0;
    scalar b = 0;

    scalar omega = 2*Foam::constant::mathematical::pi*frequency_;

    // Calculate forcing (ODE RHS), as function of pressure
    const scalar Q = calcQ(p);

    for (int i=0; i<odeSubSteps_; i++)
    {
        b = dT*( Q-sqr(omega)*a_0-2*damping_*omega*b_0 )+b_0;
        a = dT*b+a_0;

        a_0 = a; // relax*a+(1-relax)*a_0;
        b_0 = b; // relax*b+(1-relax)*b_0;
    }

    // Update Q_n-1 and Q_n-2 for later time derivative
    // if ddt Q is needed for the solver. Not used presently.
    Q_00 = Q_0;
    Q_0  = Q;

    Info << "Unit work by mode    \""<< name_ <<"\" = " << Q << endl;
    Info << "Coefficient for mode \""<< name_ <<"\" = " << a << endl;

    return a;
}

scalar structuralMode::NewmarkSolve(const volScalarField& p)
{
    scalar dT = (mesh_.time().deltaTValue());
    scalar rdT = 1.0/dT;

    scalar  q    = 0.0;             // q(n+1)
    scalar& q_0  = odeData_[0];     // q(n)
    scalar& q_00 = odeData_[1];     // q(n-1)
    scalar& Q_0  = odeData_[2];     // Q(n)
    scalar& Q_00 = odeData_[3];     // Q(n-1)

    const scalar& mMass = modeShape_.scalingFactor();

    // Mode angular velocity from Eigen frequency
    scalar omega = 2*Foam::constant::mathematical::pi*frequency_;
    scalar sqrHalfOmega = pow(0.5*omega,2);

    scalar Q   = calcQ(p);
    scalar A0  = -2*pow(rdT,2)+0.5*pow(omega,2);
    scalar Ap1 = pow(rdT,2) + damping_*omega*rdT + sqrHalfOmega;
    scalar Am1 = pow(rdT,2) - damping_*omega*rdT + sqrHalfOmega;

    scalar Qp = (Q + 2*Q_0 + Q_00)*0.25;

    q = 1/Ap1*(Qp/mMass - A0*q_0 - Am1*q_00);

    q_00 = q_0;
    q_0  = q;

    Q_00 = Q_0;
    Q_0  = Q;

    return q;
}

scalar structuralMode::forcedMotion()
{
    scalar T = mesh_.time().timeOutputValue();

    const scalar& mMass = modeShape_.scalingFactor();

    // Mode angular velocity from Eigen frequency
    scalar omega = 2*Foam::constant::mathematical::pi*frequency_;

    scalar q = mMass * Foam::sin(omega * T);

    return q;
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
