/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2006-2009 OpenCFD Ltd.
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

Class
    Foam::oceanWaveInletPhaseFvPatchScalarField

Description
    Inlet boundary condition to alpha.water derived from fixedValue.

    Calculate wave elevation at inlet according to Stokes second order
    ocean waves. This BC for alpha.water must be set together with
    oceanWaveInletVelocity BC for the U inlet BC.

    oceanWaveInletPhase and oceanWaveInletVelocity both depend on a 
    constant/oceanWavesDict file, which reads

    FoamFile
    {
        version     2.0;
        format      ascii;
        class       dictionary;
        location    "constant";
        object      oceanWavesDict;
    }
    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
    
    seaLevel                   0;  // Still water sea level
    waveHeight              0.05;
    waveLength               0.4;
    waterDepth               0.5;
    freeStreamVelocity (0.8 0 0);
    waveDirection        (1 0 0);
    
    // ************************************************************************* //


    Example of the boundary condition specification:
    @verbatim
        inlet
        {
            type        oceanWaveInletPhase;
            value       uniform 1;
        }
    @endverbatim

SourceFiles
    oceanWaveInletPhaseFvPatchScalarField.C

\*---------------------------------------------------------------------------*/

#ifndef oceanWaveInletPhaseFvPatchScalarField_H
#define oceanWaveInletPhaseFvPatchScalarField_H

#include "fixedValueFvPatchFields.H"
#include "oceanWaveFunctions.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
          Class oceanWaveInletPhaseFvPatch Declaration
\*---------------------------------------------------------------------------*/

class oceanWaveInletPhaseFvPatchScalarField
:
    public fixedValueFvPatchScalarField
{
    // Private data
    
    oceanWaveFunctions owf_;

    // Private functions
        tmp<scalarField> surfacePosition();
public:

    //- Runtime type information
    TypeName("oceanWaveInletPhase");


    // Constructors

        //- Construct from patch and internal field
        oceanWaveInletPhaseFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        oceanWaveInletPhaseFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given
        //  oceanWaveInletPhaseFvPatchScalarField
        //  onto a new patch
        oceanWaveInletPhaseFvPatchScalarField
        (
            const oceanWaveInletPhaseFvPatchScalarField&,
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct as copy
        oceanWaveInletPhaseFvPatchScalarField
        (
            const oceanWaveInletPhaseFvPatchScalarField&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchScalarField> clone() const
        {
            return tmp<fvPatchScalarField>
            (
                new oceanWaveInletPhaseFvPatchScalarField
                (
                    *this
                )
            );
        }

        //- Construct as copy setting internal field reference
        oceanWaveInletPhaseFvPatchScalarField
        (
            const oceanWaveInletPhaseFvPatchScalarField&,
            const DimensionedField<scalar, volMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<fvPatchScalarField> clone
        (
            const DimensionedField<scalar, volMesh>& iF
        ) const
        {
            return tmp<fvPatchScalarField>
            (
                new oceanWaveInletPhaseFvPatchScalarField
                (
                    *this,
                    iF
                )
            );
        }


    // Member functions

        //- Update the coefficients associated with the patch field
        virtual void updateCoeffs();

        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
