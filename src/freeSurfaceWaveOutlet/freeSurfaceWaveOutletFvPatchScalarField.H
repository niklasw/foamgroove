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

Class
    Foam::freeSurfaceWaveOutletFvPatchScalarField

Group
    grpGenericBoundaryConditions grpWallBoundaryConditions

Description
    This BC, derived from fixedGradientFvPatchField, calculates the
    outflow gradient of the volume of fluids alpha1 field from
    the near cell gradient dot outflow normal.

SourceFiles
    freeSurfaceWaveOutletFvPatchScalarField.C

\*---------------------------------------------------------------------------*/

#ifndef freeSurfaceWaveOutletFvPatchScalarFields_H
#define freeSurfaceWaveOutletFvPatchScalarFields_H

#include "fvPatchFields.H"
#include "fixedGradientFvPatchFields.H"
#include "fvc.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
             Class freeSurfaceWaveOutletFvPatchScalarField Declaration
\*---------------------------------------------------------------------------*/

class freeSurfaceWaveOutletFvPatchScalarField
:
    public fixedGradientFvPatchScalarField
{
    // Private data

public:

    //- Runtime type information
    TypeName("freeSurfaceWaveOutlet");


    // Constructors

        //- Construct from patch and internal field
        freeSurfaceWaveOutletFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        freeSurfaceWaveOutletFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given
        //  freeSurfaceWaveOutletFvPatchScalarField onto a new patch
        freeSurfaceWaveOutletFvPatchScalarField
        (
            const freeSurfaceWaveOutletFvPatchScalarField&,
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct as copy
        freeSurfaceWaveOutletFvPatchScalarField
        (
            const freeSurfaceWaveOutletFvPatchScalarField&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchScalarField> clone() const
        {
            return tmp<fvPatchScalarField>
            (
                new freeSurfaceWaveOutletFvPatchScalarField(*this)
            );
        }

        //- Construct as copy setting internal field reference
        freeSurfaceWaveOutletFvPatchScalarField
        (
            const freeSurfaceWaveOutletFvPatchScalarField&,
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
                new freeSurfaceWaveOutletFvPatchScalarField(*this, iF)
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
