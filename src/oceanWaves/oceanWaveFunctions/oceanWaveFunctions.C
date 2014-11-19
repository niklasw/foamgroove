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

Description
    Foam::oceanWaveFunctions

SourceFiles
    oceanWaveFunctions.C

\*---------------------------------------------------------------------------*/

#include "oceanWaveFunctions.H"
#include "uniformDimensionedFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

scalar oceanWaveFunctions::smoothStep
(
        const scalar s0,
        const scalar s1,
        scalar x
) const
{
    scalar out = 1;
    if ((s1 - s0) > SMALL)
    {
        x = min(max(x,s0),s1);
        x = (x - s0)/(s1 - s0);
        out = pow(x,3)*(x*(x*6 - 15) + 10);
    }
    return out;
}

oceanWaveFunctions::oceanWaveFunctions(const objectRegistry& db)
:
    db_(db)
{
    IOdictionary oceanWavesDict
    (
        Foam::IOobject
        (
            "oceanWavesDict",
            db_.time().caseConstant(),
            db_,
            IOobject::MUST_READ
        )
    );

    seaLevel_ = oceanWavesDict.lookupOrDefault("seaLevel",scalar(0));
    waveHeight_ = oceanWavesDict.lookupOrDefault("waveHeight",scalar(1));
    waveLength_ = oceanWavesDict.lookupOrDefault("waveLength",scalar(20));
    waterDepth_ = oceanWavesDict.lookupOrDefault("waterDepth",scalar(10.0));
    waveDirection_ = oceanWavesDict.lookupOrDefault("waveDirection",vector(1,0,0));
    freeStreamVelocity_ =
         oceanWavesDict.lookupOrDefault("freeStreamVelocity",vector(0,0,0));
    rampStart_ = oceanWavesDict.lookupOrDefault("rampStart",scalar(0.0));
    rampDuration_ = oceanWavesDict.lookupOrDefault("rampDuration",scalar(0.0));

    Info<< "\nReading g" << endl;
    uniformDimensionedVectorField g
    (
        IOobject
        (
            "g",
            db_.time().caseConstant(),
            db_,
            IOobject::MUST_READ
        )
    );

    g_ = g.value();

    info();
}

void oceanWaveFunctions::info() const
{
    Info << "oceanWaveFunctions::info()\n"
         << "\n - sea level      = " << seaLevel_
         << "\n - wave height    = " << waveHeight_
         << "\n - wave length    = " << waveLength_
         << "\n - wave number    = " << waveNumber()
         << "\n - wave direction = " << waveDirection_
         << "\n - phase velocity = " << celerity() 
         << "\n - omega          = " << omega() 
         << "\n - wave period    = " << period() << nl << endl;
}

scalar oceanWaveFunctions::waveNumber() const
{
    return 2*pi()/waveLength_;
}

scalar oceanWaveFunctions::ramplitude(const scalar t) const
{
    return 0.5*waveHeight_
         * smoothStep(rampStart_,rampStart_+rampDuration_,t);
    
}

scalar oceanWaveFunctions::celerity() const
{
    return sqrt(mag(g_)*waveLength_/(2*pi())*tanh(2*pi()*waterDepth_/waveLength_));
}

scalar oceanWaveFunctions::omega() const
{
    return celerity()*waveNumber();
}

scalar oceanWaveFunctions::period() const
{
    return 2*pi()/omega();
}

scalar oceanWaveFunctions::elevation
(
    const scalar t,
    const scalar x
)
{
    scalar A = ramplitude(t);
    scalar k = waveNumber();
    scalar kh = k*waterDepth_;
    scalar w = omega();

    scalar Phi = k*x - w*t;

    scalar h;
    scalar sigma = tanh(kh);

    h= A*(cos(Phi)+k*A*(3-pow(sigma,2))/(4*pow(sigma,3))*cos(2*Phi));

    /*
    h = A*cos(w*t)
        + sqr(A)*k*cosh(kh)/(4*pow(sinh(2*kh),3))
        * (2+cosh(2*kh))*cos(2*(w*t));
    */

    return h;
}

vector oceanWaveFunctions::waveVelocities
(
    const scalar x,
    const scalar z,
    const scalar t

)
{
    scalar A = ramplitude(t);
    scalar g = mag(g_);
    scalar k = waveNumber();
    scalar kh = k*waterDepth_;
    scalar w = omega();
    scalar kzh = k*(z+waterDepth_);

    scalar Phi = k*x - w*t;

    scalar ux;
    scalar uz;

    ux = A*g*k
            * cosh(kzh)/(w*cosh(kh))
            * cos(Phi)
       + sqr(A)*w*k
            * cosh(2*kzh)/pow(sinh(kh),4)
            * cos(2*(Phi));

    uz = A*g*k
            * sinh(kzh)/(w*sinh(kh))
            * sin(Phi)
       + sqr(A)*w*k
            * sinh(2*kzh)/pow(cosh(kh),4)
            * sin(2*(Phi));

    return freeStreamVelocity()
         + ux * waveDirection()
         + uz * up();
}

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// ************************************************************************* //

