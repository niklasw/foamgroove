/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2013 OpenFOAM Foundation
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
    Foam::modeShape

Description

SourceFiles
    modeShapeI.H
    modeShape.C
    modeShapeIO.C

\*---------------------------------------------------------------------------*/

#ifndef modeShape_H
#define modeShape_H
#include "primitiveFields.H"
#include "pointFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration of classes
class modalDisplacementPointPatchField;
class Istream;
class Ostream;

/*---------------------------------------------------------------------------*\
                         Class modeShape Declaration
\*---------------------------------------------------------------------------*/

class modeShape
{
    // Private data

    const dictionary& dict_;
    word modeTypeName_;

    scalar scalingFactor_;
    vector origin_;
    vector axis_;
    vector amplitude_;
    scalar waveLength_;

    const modalDisplacementPointPatchField& BC_;
    const pointField& points_;
    vectorField displacement_;

    // Private Member Functions

        //- Disallow default bitwise copy construct
        modeShape(const modeShape&);

        //- Disallow default bitwise assignment
        void operator=(const modeShape&);

        //- Mode generation functions

        // - Coordinate for analytical mode generation.
        const scalarField X();

        void genRigidMode();

        void genTrigonometricMode();

        void genPolynomialMode();

        void genInterpolatedMode();

        void distributeParModeDisplacement();

public:

    // Static data members


    // Constructors

        //- Construct null
        modeShape
        (
            const dictionary& dict,
            const modalDisplacementPointPatchField& BC
        );


    //- Destructor
    ~modeShape();


    // Member Functions

        // Access
        const vectorField& displacement() const;
        const scalar& scalingFactor() const;
        scalar& scalingFactor();

        // Check

        // Edit
        void generate();

        // Write


    // Member Operators


    // Friend Functions

    // Friend Operators

    // IOstream Operators

        friend Istream& operator>>(Istream&, modeShape&);
        friend Ostream& operator<<(Ostream&, const modeShape&);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
