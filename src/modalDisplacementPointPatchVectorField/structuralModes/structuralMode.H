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
    Foam::structuralMode

Description
    MRF zone definition based on cell zone and parameters
    obtained from a control dictionary constructed from the given stream.

    The rotation of the MRF region is defined by an origin and axis of
    rotation and an angular speed.

SourceFiles
    structuralMode.C

\*---------------------------------------------------------------------------*/

#ifndef structuralMode_H
#define structuralMode_H

#include "dictionary.H"
#include "wordList.H"
#include "labelList.H"
#include "polyMesh.H"
#include "pointPatch.H"
#include "dimensionedScalar.H"
#include "dimensionedVector.H"
#include "vectorList.H"
#include "volFieldsFwd.H"
#include "surfaceFieldsFwd.H"
#include "fvMatricesFwd.H"
#include "mapPolyMesh.H"
#include "DataEntry.H"
#include "autoPtr.H"

#include "modalDisplacementPointPatchVectorField.H"
#include "modeShape.H"
#include "myODE.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration of classes
class polyMesh;
class modalDisplacementPointPatchField;

/*---------------------------------------------------------------------------*\
                           Class structuralMode Declaration
\*---------------------------------------------------------------------------*/

class structuralMode
{
    // Private data

        const modalDisplacementPointPatchField& BC_;

        const pointPatch& patch_;

        const polyMesh& mesh_;

        const word name_;

        const dictionary dict_;

        List<scalar> odeData_;
        label odeSubSteps_;

        /*
         * Origin, axis and waveLength is only used to generate
         * trigonometric fake modes for testing. I.e. when
         * trigonometricMode is yes.
         */

        modeShape modeShape_;

        dictionary modeShapeDict_;

        //- Mode frequency
        const scalar frequency_;

        //- Modes structural damping coefficient
        const scalar damping_;

        //- Renormalisation factor
        scalar scalingFactor_;

        //- scalarField of volumes swept by faces when
        //  displaced by the eigen mode.
        scalarField sweptVols_;

        //- Ode solver
        myODE motionEquation_;

        autoPtr<ODESolver> odeSolver_;// = ODESolver::New(args[1], ode);

    // Private Member Functions

        //- Disallow default bitwise copy construct
        structuralMode(const structuralMode&);

        void calculateSweptVols();

        //- Disallow default bitwise assignment
        void operator=(const structuralMode&);

public:

    // Declare name of the class and its debug switch
    ClassName("structuralMode");


    // Constructors

        //- Construct from pointPatch, polyMesh and Istream
        //- polyMesh needed to get pointProcAddressing for now.
        structuralMode
        (
            const modalDisplacementPointPatchField& BC,
            Istream& is
        );

        //- Return clone (needed by inew I believe. Does nothing.)
        autoPtr<structuralMode> clone() const
        {
            notImplemented("autoPtr<structuralMode> clone() const");
            return autoPtr<structuralMode>(NULL);
        }

        //- Return a pointer to a new structuralMode created on freestore
        //  from Istream
        class iNew
        {
            const modalDisplacementPointPatchField& BC_;

        public:

            iNew(const modalDisplacementPointPatchField& BC)
            :
                BC_(BC)
            {}

            autoPtr<structuralMode> operator()(Istream& is) const
            {
                return autoPtr<structuralMode>
                (
                    new structuralMode(BC_,is)
                );
            }
        };


    //- Destructor
    virtual ~structuralMode()
    {}

    //- Work issued by pressure when displacing
    //  patch by mode.
    scalar Q(const volScalarField& p) const;

    //- Calculate this modes coefficient by solving
    //  the modal analysis ODE with pressure as driving
    //  force.
    scalar structuralMode::solveMotionEquation(const volScalarField& p);

    scalar calculateCoeff(const volScalarField& p);

    // Access
    
    const modeShape& shape() const
    {
        return modeShape_;
    };

    const vectorField& modeDisplacement() const
    {
        return modeShape_.displacement();
    };

    vectorField modeDisplacement(const scalar& scale) const
    {
        return scale*modeShape_.displacement();
    };

    const scalarField& sweptVols() const
    {
        return sweptVols_;
    };

    const word name() const
    {
        return name_;
    };

    const scalar& frequency() const
    {
        return frequency_;
    }

    const List<scalar>& odeData() const
    {
        return odeData_;
    }

    //- Non const reference to odeData_ in order to
    //  update odeData_ from disk, if needed.
    List<scalar>& odeData()
    {
        return odeData_;
    }

    void write() const;

    // IOstream operator

        friend Ostream& operator<<(Ostream& os, const structuralMode& mode);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


#endif

// ************************************************************************* //
