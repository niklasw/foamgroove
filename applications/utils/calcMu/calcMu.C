/*---------------------------------------------------------------------------* \
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

Application
    vorticity

Description
    Calculates and writes the vorticity of velocity field U.

    The -noWrite option just outputs the max/min values without writing
    the field.

\*---------------------------------------------------------------------------*/

#include "calc.H"
#include "fvc.H"
#include "psiThermo.H"
#include "rhoThermo.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

void Foam::calc(const argList& args, const Time& runTime, const fvMesh& mesh)
{
    bool writeResults = !args.optionFound("noWrite");

    autoPtr<rhoThermo> pThermo
    (
        rhoThermo::New(mesh)
    );
    rhoThermo& thermo = pThermo();

    IOobject Theader
    (
        "T",
        runTime.timeName(),
        mesh,
        IOobject::MUST_READ
    );

    if (Theader.headerOk())
    {
        Info<< "    Reading T" << endl;
        volScalarField T(Theader, mesh);

        Info<< "    Calculating transport properties" << endl;
        volScalarField mu
        (
            IOobject
            (
                "mu",
                runTime.timeName(),
                mesh,
                IOobject::NO_READ
            ),
            thermo.mu()
        );

        volScalarField rho
        (
            IOobject
            (
                "rho",
                runTime.timeName(),
                mesh,
                IOobject::NO_READ
            ),
            thermo.rho()
        );

        Info<< "Temperature max/min : "
            << max(T).value() << " "
            << min(T).value() << endl;

        Info<< "Dynamic viscosity max/min : "
            << max(mu).value() << " "
            << min(mu).value() << endl;

        Info<< "Density max/min : "
            << max(rho).value() << " "
            << min(rho).value() << endl;

        if (writeResults)
        {
            mu.write();
        }
    }
    else
    {
        Info<< "    No U" << endl;
    }

    Info<< "\nEnd\n" << endl;
}


// ************************************************************************* //
