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

Class
    Foam::oceanWaveInletVelocityFvPatchVectorField

Description
    Foam::oceanWaveInletVelocityFvPatchVectorField

SourceFiles
    oceanWaveInletVelocityFvPatchVectorField.C

\*---------------------------------------------------------------------------*/

#ifndef oceanWaveInletVelocityFvPatchVectorField_H
#define oceanWaveInletVelocityFvPatchVectorField_H

#include "fixedValueFvPatchFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                 Class oceanWaveInletVelocityFvPatch Declaration
\*---------------------------------------------------------------------------*/

class oceanWaveInletVelocityFvPatchVectorField
:
    public fixedValueFvPatchVectorField
{
    // Private data

        scalar seaLevel_;
        scalar waveHeight_;
        scalar waveLength_;
        scalar waterDepth_;
        vector waveDirection_;
        scalar waveVelocity_;
        vector freeStreamVelocity_;
        vector g_;

    // Private functions
        vectorField inletVelocity();
public:

    //- Runtime type information
    TypeName("oceanWaveInletVelocity");


    // Constructors

        //- Construct from patch and internal field
        oceanWaveInletVelocityFvPatchVectorField
        (
            const fvPatch&,
            const DimensionedField<vector, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        oceanWaveInletVelocityFvPatchVectorField
        (
            const fvPatch&,
            const DimensionedField<vector, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given oceanWaveInletVelocityFvPatchVectorField
        //  onto a new patch
        oceanWaveInletVelocityFvPatchVectorField
        (
            const oceanWaveInletVelocityFvPatchVectorField&,
            const fvPatch&,
            const DimensionedField<vector, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct as copy
        oceanWaveInletVelocityFvPatchVectorField
        (
            const oceanWaveInletVelocityFvPatchVectorField&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchVectorField> clone() const
        {
            return tmp<fvPatchVectorField>
            (
                new oceanWaveInletVelocityFvPatchVectorField(*this)
            );
        }

        //- Construct as copy setting internal field reference
        oceanWaveInletVelocityFvPatchVectorField
        (
            const oceanWaveInletVelocityFvPatchVectorField&,
            const DimensionedField<vector, volMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<fvPatchVectorField> clone
        (
             const DimensionedField<vector, volMesh>& iF
        ) const
        {
            return tmp<fvPatchVectorField>
            (
                new oceanWaveInletVelocityFvPatchVectorField(*this, iF)
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
