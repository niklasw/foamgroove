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
    Foam::structuralModes

Description
    Container class for a set of structuralModes with the structuralMode member functions
    implemented to loop over the functions for each structuralMode.

SourceFiles
    structuralModes.C

\*---------------------------------------------------------------------------*/

#ifndef structuralModes_H
#define structuralModes_H

#include "structuralMode.H"
#include "IOPtrList.H"
#include "volFieldsFwd.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration of classes
class modalDisplacementPointPatchField;
class structuralMode;

/*---------------------------------------------------------------------------*\
                           Class structuralModes Declaration
\*---------------------------------------------------------------------------*/

class structuralModes
:
    public IOPtrList<structuralMode>
{
    // Private data

        //- Reference to mesh
        const modalDisplacementPointPatchField& BC_;

        const vectorField& patchPoints_;

        dimensionedScalar fluidDensity_;

        word odeSolver_;

    // Private Member Functions

        //- Disallow default bitwise copy construct
        structuralModes(const structuralModes&);

        //- Disallow default bitwise assignment
        void operator=(const structuralModes&);

public:

    // Constructors

        //- Construct from polyMesh
        structuralModes(const modalDisplacementPointPatchField& BC);

    // Member Functions

        vectorField modeDisplacement() const;
        vectorField calculatedModeDisplacement(const volScalarField& p) const;

        const dimensionedScalar& fluidDensity() const
        {
            return fluidDensity_;
        };

    // I-O

        //- Read from Istream
        virtual bool readData(Istream&);

        void writeODEData() const;

        //- IF ODEData-file for the associated patch is found
        //  in the instant directory, read and distribute odeData
        //  to all modes. If no file present; Do nothing.
        void readODEData();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
