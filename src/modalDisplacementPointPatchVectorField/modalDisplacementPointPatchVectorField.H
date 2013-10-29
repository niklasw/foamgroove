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
    Foam::modalDisplacementPointPatchField

Description
    Foam::modalDisplacementPointPatchField

    boundary condition example for the pointDisplacement field:
    "wall_rod"
    {
        type                modalDisplacement;
        rampBegin          100; // Do not act until after 100 dt.
        rampEnd            1000; // Perform smooth ramp to 1000 dt.
        value               (0 0 0);
    }


    ---------------------------------------------------------------------
    constant/dynamicMeshDict example:

    dynamicFvMesh      dynamicMotionSolverFvMesh;

    motionSolverLibs ("libfvMotionSolvers.so");

    solver            displacementLaplacian;

    diffusivity       inverseDistance ("wall_rod");

    ---------------------------------------------------------------------

    ---------------------------------------------------------------------
    Also, most importantly, reads a structuralMode dictionary
    specific to the patch:

    constant/structuralModes/modeData_wall_rod

        3
        (
            first
            {
                generateMode yes;
                generatedMode
                {
                    origin  (0 0 0);
                    axis    (0 0 1);
                    waveLength   1;
                    amplitude (0.1 0 0);
                }

                frequency   2.5;
                scalingFactor 1;
                modeDisplacement 0();
            }

            second
            { ... }
       );

    ---------------------------------------------------------------------

    ---------------------------------------------------------------------
    If an incompressible solver is used, the fluid density
    needs specification:

    constant/structuralModes/fluidProperties

    fluidDensity              fluidDensity [ 1 -3 0 0 0 0 0 ] 1000;

    ---------------------------------------------------------------------


    ---------------------------------------------------------------------
    For parallel runs, a point label map for the patch needs to be
    created before decomposePar (serial run, alas!). For now, this is
    done with the application "writePatchPointLabels" included in this
    folder:

    writePatchPointLabels -patches "(wall_rod)"

    ---------------------------------------------------------------------

SourceFiles
    modalDisplacementPointPatchField.C
    structuralModes/structuralModes.C,H
    structuralModes/structuralMode.C,H

\*---------------------------------------------------------------------------*/

#ifndef modalDisplacementPointPatchField_H
#define modalDisplacementPointPatchField_H

#include "fixedValuePointPatchField.H"
#include "fvCFD.H"
#include "mathematicalConstants.H"
#include "structuralModes.H"
#include "structuralMode.H"
#include "HashTable.H"
#include "Tuple2.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

    class structuralModes;
    class structuralMode;
/*---------------------------------------------------------------------------*\
        Class modalDisplacementPointPatchField Declaration
\*---------------------------------------------------------------------------*/

class modalDisplacementPointPatchField
:
    public fixedValuePointPatchField<vector>
{
    // Private data

        label waitNSteps_;
        label rampBegin_;
        label rampEnd_;

        bool writeDebugField_;

        //- Map from global point label to point index
        //  in the serial version of this patch. Size
        //  is zero if not parallel.
        HashTable<label,label> globalMeshToGlobalPatchIndexMap_;

        List<label> patchPointProcAddressing_;

        List<Tuple2<label,label> > myMonitorMeshLabels_;

        const polyMesh *meshPtr_;

        //- As pointer because seems I need to
        //  avoid initiation in constructor 2,
        //  for decomposePar to work...???
        structuralModes *modesPtr_;

    // Private member functions

        //- A smooth Heaviside.
        scalar smoothStep
        (
            const scalar s0,
            const scalar s1,
            const scalar x
        ) const;
        //- We need a way to quickly get the index of a global points
        //  label in this patch, so create a HashTable at construction.

        void readPointProcAddressing();

        void createSerialIndexMap();

        void initIndexMaps()
        {
            createSerialIndexMap();
            readPointProcAddressing();
        };

        //- Find closest point to coordinate p. Uses gather/scatter
        //  and returns processor and mesh label (local to processor)
        //  of the closest point foind.
        Tuple2<label,label> primitiveNearestPointSearch(const point& p);

        //- Reads structuralModes/monitorPoints_<patch name> for probe
        //  coordinates and returns list of tuples with process number
        //  and mesh label of closes point (local to processor).
        List<Tuple2<label,label> > findMonitorPoints();

        void findMyMonitors();
public:

    //- Runtime type information
    TypeName("modalDisplacement");


    // Constructors

        //- Construct from patch and internal field
        modalDisplacementPointPatchField
        (
            const pointPatch&,
            const DimensionedField<vector, pointMesh>&
        );

        //- Construct from patch, internal field and dictionary
        modalDisplacementPointPatchField
        (
            const pointPatch&,
            const DimensionedField<vector, pointMesh>&,
            const dictionary&
        );

        //- Construct by mapping given patchField<vector> onto a new patch
        modalDisplacementPointPatchField
        (
            const modalDisplacementPointPatchField&,
            const pointPatch&,
            const DimensionedField<vector, pointMesh>&,
            const pointPatchFieldMapper&
        );

        //- Construct and return a clone
        virtual autoPtr<pointPatchField<vector> > clone() const
        {
            return autoPtr<pointPatchField<vector> >
            (
                new modalDisplacementPointPatchField
                (
                    *this
                )
            );
        }

        //- Construct as copy setting internal field reference
        modalDisplacementPointPatchField
        (
            const modalDisplacementPointPatchField&,
            const DimensionedField<vector, pointMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual autoPtr<pointPatchField<vector> > clone
        (
            const DimensionedField<vector, pointMesh>& iF
        ) const
        {
            return autoPtr<pointPatchField<vector> >
            (
                new modalDisplacementPointPatchField
                (
                    *this,
                    iF
                )
            );
        }


    // Member functions

        //- Return the HashTable that maps Global mesh point index
        //  to index in serial version of this->patch
        const HashTable<label,label>& globalMeshToGlobalPatchIndexMap() const
        {
            return globalMeshToGlobalPatchIndexMap_;
        }

        const List<label>& pointProcAddressing() const
        {
            return patchPointProcAddressing_;
        }

        //- Return the index in this patch of a mesh index
        label localIndex(const label i) const
        {
            return meshPtr_->boundaryMesh()[this->patch().index()].whichPoint(i);
        };

        //- Patch-local index to (serial) global index (through pointProcAddressing)
        label globalMeshIndex(const label i) const
        {
            if (i > patchPointProcAddressing_.size())
            {
                FatalErrorIn("modalDisplacementPointPatchField::globalMeshIndex")
                    << "List index exceeds patchPointProcAddressing_ size"
                    << exit(FatalError);
            }
            return patchPointProcAddressing_[i];
        };

        //- Maps global mesh index to global (serial) patch index
        label globalMeshToGlobalPatchIndex(const label i) const
        {
            return globalMeshToGlobalPatchIndexMap_[i];
        };

        //- Maps local patch index to mesh index
        label meshIndex(const label i) const
        {
            return this->patch().meshPoints()[i];
        };

        //- Maps local patch index to global patch index.
        //  (Wraps two mappings, for convenience)
        label localToGlobalPatchIndex(const label i) const
        {
            return globalMeshToGlobalPatchIndex(globalMeshIndex(i));
        }

        // Evaluation functions

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
