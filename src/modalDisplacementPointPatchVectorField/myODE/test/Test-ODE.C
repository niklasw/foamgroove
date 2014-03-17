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

Description

\*---------------------------------------------------------------------------*/

#include "argList.H"
#include "IOmanip.H"
#include "ODESolver.H"
#include "mathematicalConstants.H"
#include "myODE.H"

using namespace Foam;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
// Main program:

int main(int argc, char *argv[])
{
    argList::validArgs.append("ODESolver");
    argList args(argc, argv);

    scalar twoPi = 2*Foam::constant::mathematical::pi;
    scalar freq  = 4;
    scalar omega = freq*twoPi;

    scalar damping = 0.0;

    dictionary dict;

    dict.add("solver", args[1]);

    myODE ode(freq,damping);
    autoPtr<ODESolver> odeSolver = ODESolver::New(ode, dict);

    scalar xStart = 0.0;
    scalarField yStart(ode.nEqns());

    yStart[0] = 0;
    yStart[1] = 0;

    scalarField dyStart(ode.nEqns());

    ode.derivatives(xStart, yStart, dyStart);

    scalar x = xStart;
    scalar xEnd = x + 4.0;

    scalar dt = 1e-3;

    while (x < xEnd)
    {
        scalar q = Foam::sin(x*omega*1.1);
        ode.RHS(q);
        scalarField y(yStart);
        scalar hEst = dt;
        odeSolver->relTol() = 1e-5;
        odeSolver->solve(x, x+dt, y, hEst);
        x += dt;

        Info <<  x <<" " << q <<" "<< y[0] << " " << y[1] << " " <<  hEst << endl;
    }


    Info<< "\nEnd\n" << endl;

    return 0;
}


// ************************************************************************* //
