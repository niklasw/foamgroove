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
    oceanWaveInletVelocityFvPatchVectorField.C

\*---------------------------------------------------------------------------*/

#ifndef oceanWaveFunctions_H
#define oceanWaveFunctions_H

#include "mathematicalConstants.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

class oceanWaveFunctions
{
    private:
        const fvMesh& mesh_;
        scalar seaLevel_;
        scalar waveHeight_;
        scalar waveLength_;
        scalar waterDepth_;
        vector waveDirection_;
        scalar waveVelocity_;
        vector freeStreamVelocity_;
        vector g_;

    public:
        oceanWaveFunctions(const fvMesh& mesh);

        scalar waveNumber(const scalar waveLength);

        scalar celerity
        (
            const scalar waveLength,
            const scalar waterDepth,
            const scalar g
        );

        scalar elevation
        (
            const scalar waveLength,
            const scalar waveHeight,
            const scalar waterDepth,
            const scalar g,
            const scalar t,
            const scalar x
        );

        void waveVelocities
        (
            const scalar waveLength,
            const scalar waveHeight,
            const scalar waterDepth,
            const scalar g,
            const scalar t,
            const scalar x,
            const scalar z,
            const scalar& ux,
            const scalar& uz
        );

};


oceanWaveFunctions::oceanWaveFunctions(const fvMesh& mesh)
:
    mesh_(mesh)
{
    IOdictionary oceanWavesDict
    (
        Foam::IOobject
        (
            "oceanWavesDict",
            mesh.time().caseConstant(),
            mesh,
            IOobject::MUST_READ
        )
    );
    seaLevel_ = oceanWavesDict.lookupOrDefault("seaLevel",scalar(0));
    waveHeight_ = oceanWavesDict.lookupOrDefault("waveHeight",scalar(1));
    waveLength_ = oceanWavesDict.lookupOrDefault("waveLength",scalar(20));
    waterDepth_ = oceanWavesDict.lookupOrDefault("waterDepth",scalar(10.0));
    waveDirection_ = oceanWavesDict.lookupOrDefault("waveDirection",vector(1,0,0));
}

scalar oceanWaveFunctions::waveNumber(scalar waveLength)
{
    const scalar pi = constant::mathematical::pi;
    return 2*pi/waveLength;
}

scalar oceanWaveFunctions::celerity
(
    const scalar waveLength,
    const scalar waterDepth,
    const scalar g
)
{
    const scalar pi = constant::mathematical::pi;
    return sqrt(g*waveLength/(2*pi)*tanh(2*pi*waterDepth/waveLength)); //Checked ok.
}

scalar oceanWaveFunctions::elevation
(
    const scalar waveLength,
    const scalar waveHeight,
    const scalar waterDepth,
    const scalar g,
    const scalar t,
    const scalar x
)
{
    scalar A = waveHeight/2;
    scalar k = waveNumber(waveLength);
    scalar kh = k*waterDepth;
    scalar omega = celerity(waveLength,waterDepth,g)*k;

    scalar h;

    h = A*cos(omega*t)
      + sqr(A)*k*cosh(kh)/(4*pow(sinh(2*kh),3))
      * (2+cosh(2*kh))*cos(2*(omega*t));

    return h;
}

void oceanWaveFunctions::waveVelocities
(
    const scalar waveLength,
    const scalar waveHeight,
    const scalar waterDepth,
    const scalar g,
    const scalar t,
    const scalar x,
    const scalar z,
    scalar& ux,
    scalar& uz

)
{
    scalar A = waveHeight/2;
    scalar k = waveNumber(waveLength);
    scalar kh = k*waterDepth;
    scalar omega = celerity(waveLength,waterDepth,g)*k;
    scalar zh = z+waterDepth;

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

    return;
}

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
