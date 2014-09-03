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

\*---------------------------------------------------------------------------*/

#include "gunTopoFvMesh.H"
#include "Time.H"
#include "mapPolyMesh.H"
#include "layerAdditionRemoval.H"
#include "addToRunTimeSelectionTable.H"
#include "meshTools.H"
#include "OFstream.H"
#include "pointFields.H"
#include "volFields.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(gunTopoFvMesh, 0);

    addToRunTimeSelectionTable
    (
        topoChangerFvMesh,
        gunTopoFvMesh,
        IOobject
    );
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::gunTopoFvMesh::debugInfo()
{
    Info << "\tBullet info: "
         <<"time:" << time().value()
         << " curMotionVel_:" << curMotionVel_
         << " growthLayerPosition:" << growthLayerPosition_ << endl;
    Info << "\tBullet info: "
         << "Current volume of domain = " << gSum(V()) << endl;

    return;
}

void Foam::gunTopoFvMesh::updateExtrudeLayerPosition()
{
    label zoneI = faceZones().findZoneID("extrusionFaces");
    growthLayerPosition_ = average
    (
        faceZones()[zoneI]().localPoints().component(vector::X)
    );
}

Foam::tmp<Foam::scalarField> Foam::gunTopoFvMesh::vertexMarkup
(
    const pointField& p
) const
{
    tmp<scalarField> tvertexMarkup(new scalarField(p.size()));
    scalarField& vertexMarkup = tvertexMarkup();

    forAll(p, pI)
    {
        if (p[pI].x() > growthLayerPosition_+tolerance_) // CHECK DIRECTION!
        {
            vertexMarkup[pI] = 1;
        }
        else
        {
            vertexMarkup[pI] = 0;
        }
    }

    Info << "\tPiston info: "
         << "Number of vertices marked for motion = "
         << gSum(vertexMarkup) << endl;

    return tvertexMarkup;
}


void Foam::gunTopoFvMesh::addZonesAndModifiers()
{
    // Add zones and modifiers for motion action

    if
    (
        pointZones().size()
     || faceZones().size()
     || cellZones().size()
     || topoChanger_.size()
    )
    {
        Info<< "void gunTopoFvMesh::addZonesAndModifiers() : "
            << "Zones and modifiers already present.  Skipping."
            << endl;

        return;
    }

    Info<< "Time = " << time().timeName() << endl
        << "Adding zones and modifiers to the mesh" << endl;

    const vectorField& fc = faceCentres();
    const vectorField& fa = faceAreas();

    labelList zone1(fc.size());
    boolList flipZone1(fc.size(), false);
    label nZoneFaces1 = 0;

    forAll(fc, faceI)
    {
        if (mag(growthLayerPosition_ - fc[faceI].x()) < tolerance_)
        {
            if ((fa[faceI] & aimVector_) < 0)
            {
                flipZone1[nZoneFaces1] = true;
            }

            zone1[nZoneFaces1] = faceI;
            // Info<< "face " << faceI << " for zone 1.  Flip: "
            //     << flipZone1[nZoneFaces1] << endl;
            nZoneFaces1++;
        }
    }

    zone1.setSize(nZoneFaces1);
    flipZone1.setSize(nZoneFaces1);

    // Info<< "zone: " << zone1 << endl;

    List<pointZone*> pz(0);
    List<cellZone*> cz(0);
    List<faceZone*> fz(1);

    fz[0] =
        new faceZone
        (
            "extrusionFaces",
            zone1,
            flipZone1,
            0,
            faceZones()
        );

    Info<< "Adding mesh zones." << endl;
    addZones(pz, fz, cz);


    // Add layer addition/removal interfaces

    List<polyMeshModifier*> tm(1);

    tm[0] =
        new layerAdditionRemoval
        (
            "extrusion",
            0,
            topoChanger_,
            "extrusionFaces",
            readScalar
            (
                motionDict_.subDict("extrusion").lookup("minThickness")
            ),
            readScalar
            (
                motionDict_.subDict("extrusion").lookup("maxThickness")
            )
        );

    Info<< "Adding mesh modifier" << endl;
    topoChanger_.addTopologyModifiers(tm);

    write();
}

void Foam::gunTopoFvMesh::updateMappedFaces(const mapPolyMesh& topoChangeMap )
{
        PackedBoolList mappedFace(nFaces());

        const label nOldInternal = topoChangeMap.oldPatchStarts()[0];

        const labelList& faceMap = topoChangeMap.faceMap();
        for (label faceI = 0; faceI < nInternalFaces(); faceI++)
        {
            if (faceMap[faceI] >= 0)
            {
                mappedFace[faceI] = 1;
            }
        }
        for (label faceI = nInternalFaces(); faceI < nFaces(); faceI++)
        {
            if (faceMap[faceI] >= 0 && faceMap[faceI] >= nOldInternal)
            {
                mappedFace[faceI] = 1;
            }
        }

        const List<objectMap>& fromFaces = topoChangeMap.facesFromFacesMap();

        forAll(fromFaces, i)
        {
            mappedFace[fromFaces[i].index()] = 1;
        }

        const List<objectMap>& fromEdges = topoChangeMap.facesFromEdgesMap();

        forAll(fromEdges, i)
        {
            mappedFace[fromEdges[i].index()] = 1;
        }

        const List<objectMap>& fromPts = topoChangeMap.facesFromPointsMap();

        forAll(fromPts, i)
        {
            mappedFace[fromPts[i].index()] = 1;
        }

        // Set unmapped faces to zero
        Info<< "rawTopoChangerFvMesh : zeroing unmapped boundary values."
            << endl;
        zeroUnmappedValues<scalar, fvPatchField, volMesh>(mappedFace);
        zeroUnmappedValues<vector, fvPatchField, volMesh>(mappedFace);
        zeroUnmappedValues<sphericalTensor, fvPatchField, volMesh>(mappedFace);
        zeroUnmappedValues<symmTensor, fvPatchField, volMesh>(mappedFace);
        zeroUnmappedValues<tensor, fvPatchField, volMesh>(mappedFace);

        // Special handling for phi: set unmapped faces to recreated phi
        Info<< "rawTopoChangerFvMesh :"
            << " recreating phi for unmapped boundary values." << endl;

        if (foundObject<volVectorField>("U"))
        {
            const volVectorField& U = lookupObject<volVectorField>("U");
            surfaceScalarField& phi = const_cast<surfaceScalarField&>
            (
                lookupObject<surfaceScalarField>("phi")
            );
            setUnmappedValues
            (
                phi,
                mappedFace,
                (linearInterpolate(U) & Sf())()
            );
        }
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Construct from components
Foam::gunTopoFvMesh::gunTopoFvMesh(const IOobject& io)
:
    topoChangerFvMesh(io),
    motionDict_
    (
        IOdictionary
        (
            IOobject
            (
                "dynamicMeshDict",
                time().constant(),
                *this,
                IOobject::MUST_READ_IF_MODIFIED,
                IOobject::NO_WRITE,
                false
            )
        ).subDict(typeName + "Coeffs")
    ),
    aimVector_(motionDict_.lookup("aimVector")),
    boltPosition_(motionDict_.lookup("boltPosition")),
    barrelLength_(readScalar(motionDict_.lookup("barrelLength"))),
    initialPosition_(readScalar(motionDict_.lookup("initialPosition"))),
    initialVelocity_(readScalar(motionDict_.lookup("initialVelocity"))),
    exitVelocity_(readScalar(motionDict_.lookup("exitVelocity"))),
    curMotionVel_(curMotionVel()),
    currentPosition_(initialPosition_),
    growthLayerPosition_(readScalar(motionDict_.lookup("growthLayerPosition"))),
    tolerance_(SMALL)
    //tolerance_(readScalar(motionDict_.subDict("extrusion").lookup("tolerance")))
{
    //normalize aimVector
    if ( mag(aimVector_) <= SMALL )
    {
        FatalErrorIn("gunTopoFvMesh::gunTopoFvMesh: ")
            << "Aim vector too short."
            << abort(FatalError);
    }

    aimVector_ /= (mag(aimVector_)+SMALL);

    if
    (
        motionDict_.subDict("extrusion").readIfPresent("tolerance",tolerance_)
    )
    {
        Info << "Point search tolerance read from dict = "
             << tolerance_ << endl;
    }
    Info<< "Initial time:" << time().value()
        << " Initial curMotionVel_:" << curMotionVel_
        << endl;

    addZonesAndModifiers();

    updateExtrudeLayerPosition();

    motionMask_ = vertexMarkup(points());
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::gunTopoFvMesh::~gunTopoFvMesh()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::gunTopoFvMesh::update()
{
    // Do mesh changes (use inflation - put new points in topoChangeMap)
   autoPtr<mapPolyMesh> topoChangeMap = topoChanger_.changeMesh(true);

    // Calculate the new point positions depending on whether the
    // topological change has happened or not
    pointField newPoints;

    curMotionVel_ = curMotionVel();

    debugInfo();

    // Calculate motion points displacement.
    vectorField pointsDisplacement = motionMask_*aimVector_
                                   * curMotionVel_*time().deltaT().value();

    if (topoChangeMap.valid())
    {
        Info<< "Topology change. Calculating motion points" << endl;

        updateMappedFaces(topoChangeMap());

        if (topoChangeMap().hasMotionPoints())
        {
            Info<< "Topology change. Has premotion points" << endl;

            motionMask_ = vertexMarkup(topoChangeMap().preMotionPoints());

            // Move points inside the motionMask
            newPoints = topoChangeMap().preMotionPoints() + pointsDisplacement;
        }
        else
        {
            Info<< "Topology change. Already set mesh points" << endl;

            motionMask_ = vertexMarkup(points());

            // Move points inside the motionMask
            newPoints = points() + pointsDisplacement;
        }
    }
    else
    {
        Info<< "No topology change" << endl;
        // Set the mesh motion
        newPoints = points() + pointsDisplacement;
    }

    // The mesh now contains the cells with zero volume
    Info << "Executing mesh motion" << endl;
    movePoints(newPoints);
    //  The mesh now has got non-zero volume cells

    updateExtrudeLayerPosition();

    return true;
}

/*
Foam::vector Foam::gunTopoFvMesh::curPosition() const
{
    return A - A*Foam::cos(time().value() * 2*M_PI * frequency_);
}
*/

Foam::scalar Foam::gunTopoFvMesh::curMotionVel()
{
    /*
    scalar accelerationLength = barrelLength_-initialPosition_;
    scalar dvdx = (exitVelocity_ - initialVelocity_)/accelerationLength;
    Info << "dvdx ::: " << dvdx << endl;
    Info << "current pos ::: " << currentPosition_ << endl;
    Info << "initial pos ::: " << initialPosition_ << endl;
    curMotionVel_= min((currentPosition_ - initialPosition_)*dvdx, exitVelocity_);
    currentPosition_ +=  this->time().deltaTValue() * curMotionVel_;
    */
    return 10;// curMotionVel_;
}

// ************************************************************************* //
