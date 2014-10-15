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

#ifndef oceanWaveFunctions_H
#define oceanWaveFunctions_H

#include "meshTools.H"
#include "objectRegistry.H"
#include "IOdictionary.H"
#include "Time.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

class oceanWaveFunctions
{
    private:
        const objectRegistry& db_;
        scalar seaLevel_;
        scalar waveHeight_;
        scalar waveLength_;
        scalar waterDepth_;
        vector waveDirection_;
        vector freeStreamVelocity_;
        vector g_;

    public:
        oceanWaveFunctions(const objectRegistry& db);
        oceanWaveFunctions();
        oceanWaveFunctions(const oceanWaveFunctions& owf);
        ~oceanWaveFunctions();


        scalar waveNumber();

        scalar celerity();

        scalar omega();

        scalar period();

        scalar elevation
        (
            const scalar t,
            const scalar x
        );

        vector waveVelocities
        (
            const scalar x,
            const scalar z,
            const scalar t
        );

        vector up()
        {
            return -g_/mag(g_);
        }
        //- Access

        scalar seaLevel() { return seaLevel_; };
        scalar waveHeight() { return waveHeight_; }
        scalar waveLength() { return waveLength_; }
        scalar waterDepth() { return waterDepth_; }
        vector waveDirection() { return waveDirection_; }
        vector freeStreamVelocity() { return freeStreamVelocity_; }
        vector g() { return g_; }
};

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

