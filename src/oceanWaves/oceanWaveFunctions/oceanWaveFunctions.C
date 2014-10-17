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
#include "mathematicalConstants.H"
#include "uniformDimensionedFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

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

    Info << "oceanWaveFunctions:\n"
         << "\n - sea level      = " << seaLevel_
         << "\n - wave height    = " << waveHeight_
         << "\n - wave length    = " << waveLength_
         << "\n - wave number    = " << waveNumber()
         << "\n - wave direction = " << waveDirection_
         << "\n - phase velocity = " << celerity() 
         << "\n - wave period    = " << period() << nl << endl;

}

/* Probably, the automatic copy constructor suffice
oceanWaveFunctions::oceanWaveFunctions(const oceanWaveFunctions& owf)
:
    db_(owf.db_),
    seaLevel_(owf.seaLevel_),
    waveHeight_(owf.waveHeight_),
    waveLength_(owf.waveLength_),
    waterDepth_(owf.waterDepth_),
    waveDirection_(owf.waveDirection_),
    freeStreamVelocity_(owf.freeStreamVelocity_),
    g_(owf.g_)
{}

oceanWaveFunctions::~oceanWaveFunctions()
{}
*/

scalar oceanWaveFunctions::waveNumber()
{
    const scalar pi = constant::mathematical::pi;
    return 2*pi/waveLength_;
}

scalar oceanWaveFunctions::celerity()
{
    const scalar pi = constant::mathematical::pi;
    return sqrt(mag(g_)*waveLength_/(2*pi)*tanh(2*pi*waterDepth_/waveLength_));
}

scalar oceanWaveFunctions::omega()
{
    return celerity()*waveNumber();
}

scalar oceanWaveFunctions::period()
{
    return 2*constant::mathematical::pi/omega();
}

scalar oceanWaveFunctions::elevation
(
    const scalar t,
    const scalar x
)
{
    scalar A = waveHeight_/2;
    scalar k = waveNumber();
    scalar kh = k*waterDepth_;
    scalar w = omega();

    scalar h;

    /*
    h = A*cos(w*t)
        + sqr(A)*k*cosh(kh)/(4*pow(sinh(2*kh),3))
        * (2+cosh(2*kh))*cos(2*(w*t));
    */

    h = A*sin(w*t)
        + sqr(A)*k*sinh(kh)/(4*pow(cosh(2*kh),3))
        * (2+sinh(2*kh))*sin(2*(w*t));

    return h;
}

vector oceanWaveFunctions::waveVelocities
(
    const scalar x,
    const scalar z,
    const scalar t

)
{
    scalar A = waveHeight_/2;
    scalar g = mag(g_);
    scalar k = waveNumber();
    scalar kh = k*waterDepth_;
    scalar omega = celerity()*k;
    scalar zh = z+waterDepth_;

    /*
    ux = A*g*k
            * cos(omega*t)
            * sinh(k*zh)/(omega*cosh(kh))
       + sqr(A)*omega*k
            * cos(2*(omega*t))
            * cosh(2*k*zh)/pow(sinh(kh),4);

    uz = A*g*k
            * sin(omega*t)
            * cosh(k*(zh))/(omega*sinh(kh))
       + sqr(A)*omega*k
            * sin(2*(omega*t))
            * sinh(2*k*zh)/pow(cosh(kh),4);
    */

    scalar ux,uz;

    ux = A*g*k
            * sin(omega*t)
            * cosh(k*zh)/(omega*sinh(kh))
       + sqr(A)*omega*k
            * sin(2*(omega*t))
            * sinh(2*k*zh)/pow(cosh(kh),4);

    uz = A*g*k
            * cos(omega*t)
            * sinh(k*(zh))/(omega*cosh(kh))
       + sqr(A)*omega*k
            * cos(2*(omega*t))
            * cosh(2*k*zh)/pow(sinh(kh),4);

    return freeStreamVelocity()+ux*waveDirection()+uz*up();
}

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// ************************************************************************* //

