/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011 OpenFOAM Foundation
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
    Foam::gunFvMesh

Description
    Sample topoChangerFvMesh that moves an object in x direction
    and introduces/removes layers.

    Pre-processing needed:
    Generation of faceSet containing extrusion faces
    Generation of pointSet containing barrel points

SourceFiles
    gunFvMesh.C

\*---------------------------------------------------------------------------*/

#ifndef gunFvMesh_H
#define gunFvMesh_H

#include "dynamicFvMesh.H"
#include "motionSolver.H"
#include "pointSet.H"
#include "faceSet.H"
#include "fvCFD.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration of classes

/*---------------------------------------------------------------------------*\
                      Class gunFvMesh Declaration
\*---------------------------------------------------------------------------*/

class gunFvMesh
:
    public dynamicFvMesh
{
    // Private data

        //- Motion dictionary
        dictionary motionDict_;

        //- Barrel information
        vector aimVector_;

        const vector boltPosition_;

        const scalar barrelLength_;

        const word barrelPointSet_;
        const word motionPointSet_;

        //- Bullet information
        const scalar initialPosition_;

        const scalar initialVelocity_;

        const scalar exitVelocity_;

        //- Information bearers
        scalar curMotionVel_;

        scalar currentPosition_;

        //- Search tolerance, for finding motion points.
        scalar tolerance_;

        labelList gunPoints_;
        labelList motionPoints_;

        label nOldPoints_;


    // Private Member Functions

        //- Disallow default bitwise copy construct
        gunFvMesh(const gunFvMesh&);

        //- Disallow default bitwise assignment
        void operator=(const gunFvMesh&);

        //- Calculate current position of extrusion layer
        void debugInfo();

        void updateMappedFaces(const mapPolyMesh&);

        //- Find which points to move

        //- Get bounding box for barrel mesh
        const boundBox gunBounds(const List<label>& pointLabels) const;

        //- Markup motion vertices
        tmp<scalarField> motionMask() const;

        List<label> addedPoints(const mapPolyMesh& topoMap) const;
        //- Use in constructor, to initiate points belonging to
        //  the barrel part of the mesh
        void gunMarkup();

        void linearizeU() const;


public:

    //- Runtime type information
    TypeName("gunFvMesh");


    // Constructors

        //- Construct from database
        explicit gunFvMesh(const IOobject& io);


    //- Destructor
    virtual ~gunFvMesh();


    // Member Functions

        //- Update the mesh for both mesh motion and topology change
        virtual bool update();

        //- Current position rel to lower turn
        // vector curPosition() const;

        //- Present bullet velocity, defined positive upwards
        scalar curMotionVel();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
