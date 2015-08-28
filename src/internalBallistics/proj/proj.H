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
    Foam::proj

Description
    Projectile object for use with gunTopoChangerFvMesh.H

    Main task is to set motion velocity for the mesh, and
    this proj object facilitates several methods for motion calculation.

SourceFiles
    proj.C

\*---------------------------------------------------------------------------*/

#ifndef proj_H
#define proj_H

#include "fvMesh.H"
#include "fvCFD.H"
#include "gunTopoFvMesh.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration of classes
class gunTopoFvMesh;

class proj
{
    // Private data

        //- Motion dictionary
        dictionary projDict_;

        const gunTopoFvMesh& mesh_;

        const word motionType_;

        //- Bullet information
        scalar initialPosition_;

        scalar initialVelocity_;

        scalar exitVelocity_;

        scalar currentVelocity_;

        scalar currentPosition_;

        scalar mass_;

        scalar frictionCoeff_;

        List<word> patchNames_;
        List<label> patchLabels_;

public:

    // Constructors

        //- Construct from database
        proj(const dictionary& dict, const gunTopoFvMesh& mesh);


    //- Destructor
    virtual ~proj();


    // Member Functions

        void info() const;

        //- Update the mesh for both mesh motion and topology change
        bool update();

        void updateVelocity();

        //- Current position rel to lower turn
        scalar currentPosition() const;

        //- Present bullet velocity, defined positive upwards
        scalar currentVelocity() const;

        vector pressureForce();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


#endif

// ************************************************************************* //
