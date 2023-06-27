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
    Foam::pistonTopoFvMesh

Description
    Sample topoChangerFvMesh that moves an object in x direction
    and introduces/removes layers.

SourceFiles
    pistonTopoFvMesh.C

\*---------------------------------------------------------------------------*/

#ifndef pistonTopoFvMesh_H
#define pistonTopoFvMesh_H

#include "topoChangerFvMesh.H"
#include "motionSolver.H"
#include "fvCFD.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration of classes

/*---------------------------------------------------------------------------*\
                      Class pistonTopoFvMesh Declaration
\*---------------------------------------------------------------------------*/

class pistonTopoFvMesh
:
    public topoChangerFvMesh
{
    // Private data

        //- Motion dictionary
        dictionary motionDict_;

        //- Stroke (vector)
        const vector stroke_;

        //- Crank shaft rpm
        const scalar RPM_;

        const scalar frequency_;

        vector upVector_;

        //- Motion velocity period
        vector curMotionVel_;

        //- Start piston layer position
        scalar curLower_;

        //- Search tolerance, for finding motion points.
        scalar tolerance_;

        //- Vertex motion mask
        scalarField motionMask_;


    // Private Member Functions

        template<class Type, template<class> class PatchField, class GeoMesh>
        static void setUnmappedValues
        (
            GeometricField<Type, PatchField, GeoMesh>& fld,
            const PackedBoolList& mappedFace,
            const GeometricField<Type, PatchField, GeoMesh>& baseFld
        );

        template<class Type, template<class> class PatchField, class GeoMesh>
        void zeroUnmappedValues(const PackedBoolList&) const;

        //- Disallow default bitwise copy construct
        pistonTopoFvMesh(const pistonTopoFvMesh&);

        //- Disallow default bitwise assignment
        void operator=(const pistonTopoFvMesh&);

        //- Calculate current position of extrusion layer
        void updateExtrudeLayerPosition();

        void debugInfo();

        //- Add mixer zones and modifiers
        void addZonesAndModifiers();

        void updateMappedFaces(const mapPolyMesh&);

        //- Markup motion vertices
        tmp<scalarField> vertexMarkup(const pointField& p) const;


public:

    //- Runtime type information
    TypeName("pistonTopoFvMesh");


    // Constructors

        //- Construct from database
        explicit pistonTopoFvMesh(const IOobject& io);


    //- Destructor
    virtual ~pistonTopoFvMesh();


    // Member Functions

        //- Update the mesh for both mesh motion and topology change
        virtual bool update();

        //- Current position rel to lower turn
        vector curPosition() const;

        //- Present piston velocity, defined positive upwards
        vector curMotionVel() const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "pistonTopoFvMeshTemplates.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
