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
#include "mathematicalConstants.H"

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
        scalar rampStart_;
        scalar rampDuration_;
        vector g_;
        label curTimeIndex_;

        scalar smoothStep
        (
                const scalar s0,
                const scalar s1,
                scalar x
        ) const;

    public:
        oceanWaveFunctions(const objectRegistry& db);
        oceanWaveFunctions();

        static scalar pi()
        {
            return constant::mathematical::pi;
        };

        //- Prints general wave input information
        void info() const;

        scalar waveNumber() const;

        scalar ramplitude(const scalar t) const;

        scalar celerity() const;

        scalar relativeCelerity() const;

        scalar omega() const;

        scalar period() const;

        //- Second order Stokes approximation of wave elevation
        //  for arbitrary depth waves (waveLenth < waterDepth/2)
        scalar elevation
        (
            const scalar t,
            const scalar x
        );

        //- Second order Stokes approximation of wave induced velocity
        //  for arbitrary depth waves (waveLenth < waterDepth/2)
        vector waveVelocities
        (
            const scalar x,
            const scalar z,
            const scalar t
        );

        //- Whats up? Ask gravity.
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
        label& curTimeIndex() { return curTimeIndex_; }
};

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

