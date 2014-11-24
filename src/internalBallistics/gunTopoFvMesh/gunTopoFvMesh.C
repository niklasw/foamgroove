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
#include "SortableList.H"

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
    label zoneI = faceZones().findZoneID(extrusionFaceSet_);
    if ( zoneI >= 0 )
    {
        growthLayerPosition_ = average
        (
            faceZones()[zoneI]().localPoints().component(vector::X)
        );
    }
    else
    {
        FatalErrorIn("gunTopoFvMesh::updateExtrudeLayerPosition: ")
            << "faceZone extrusionFaces not found."
            << abort(FatalError);
    }
}


Foam::tmp<Foam::scalarField> Foam::gunTopoFvMesh::vertexMarkup
(
    const pointField& p
) const
{
    /*
     * Try this instead: Get a list of moving points
     * then a list of added points. Use the union of these
     * to create mask
     *
     * Look for the addedPointMap
     */
    tmp<scalarField> tvertexMarkup(new scalarField(p.size(),0.0));
    scalarField& vertexMarkup = tvertexMarkup();

    forAll(motionPoints_, pI)
    {
        label gP = motionPoints_[pI];
        vertexMarkup[gP] = 1;
    }

    Info << "\tFoam::gunTopoFvMesh::vertexMarkup "
         << "Number of vertices marked for motion = "
         << gSum(vertexMarkup) << endl;

    return tvertexMarkup;
}

List<label> Foam::gunTopoFvMesh::addedPoints(const mapPolyMesh& topoMap) const
{
    const List<label>& pointMap = topoMap.pointMap();
    const List<label>& revMap = topoMap.reversePointMap();

    List<label> newPts(0);

    forAll(revMap,i)
    {
        if (revMap[i] != pointMap[i])
        {
            newPts.append(revMap[i]);
        }
    }

    List<point>  newMeshPoints(newPts.size());
    forAll(newPts,i)
    {
        newMeshPoints[i] = points()[newPts[i]];
    }

    Foam::boundBox bb(newMeshPoints);

    Info << "Foam::gunTopoFvMesh::addedPoints bounding box:\n" << bb << endl;

    Info << "Foam::gunTopoFvMesh::addedPoints:\n" << newPts << endl;
    return newPts;
}

void Foam::gunTopoFvMesh::gunMarkup()
{
    pointSet barrelPoints(*this,barrelPointSet_);
    pointSet motionPoints(*this,motionPointSet_);

    gunPoints_.append(barrelPoints.toc());
    motionPoints_.append(motionPoints.toc());
}

void Foam::gunTopoFvMesh::updateGunPointLabels
(
    const labelList& revPointMap
)
{
    label nMeshPoints = revPointMap.size();

    //- Assumption: all new points will be motion points
    for ( int i=nOldPoints_; i<nMeshPoints; i++)
    {
        gunPoints_.append(revPointMap[i]);
        motionPoints_.append(revPointMap[i]);
    }
}

void Foam::gunTopoFvMesh::addZonesAndModifiers()
{
    // Add zones and modifiers for motion action

    if
    (
          pointZones().size()
       || faceZones().size()
       || cellZones().size()
    )
    {
        FatalErrorIn("gunTopoFvMesh::addZonesAndModifiers ")
            << "Bad implementation. Sorry: Existing zones must be removed."
            << abort(FatalError);
    }
    if (topoChanger_.size())
    {
        Info<< "void gunTopoFvMesh::addZonesAndModifiers() : "
            << "Modifiers already present.  Skipping."
            << endl;

        return;
    }

    Info<< "Time = " << time().timeName() << endl
        << "Adding zones and modifiers to the mesh" << endl;

    //const vectorField& fc = faceCentres();
    const vectorField& fa = faceAreas();

    // Read faceSet and create and insert initial
    // faceZone in mesh. Flipmap not OK now
    faceSet extrusionSet(*this, extrusionFaceSet_);
    SortableList<label> zone1(extrusionSet.toc());
    List<bool> flipZone1(extrusionSet.size(), false);

    forAll(zone1, faceI)
    {
        label fzI = zone1[faceI];
        Info << fa[fzI] << "\t" << aimVector_ << endl;
        if ((fa[fzI] & aimVector_) < 0)
        {
            flipZone1[faceI] = true;
        }
    }

    List<pointZone*> pz(0);
    List<cellZone*> cz(0);
    List<faceZone*> fz(1);

    fz[0] =
        new faceZone
        (
            extrusionFaceSet_,
            zone1,
            flipZone1,
            0,
            faceZones()
        );

    addZones(pz, fz, cz);

    Info << "Added mesh zone " << extrusionFaceSet_
         << ". Zone size = " << faceZones()[0].size()  << endl;

    // Add layer addition/removal interfaces

    List<polyMeshModifier*> tm(1);

    tm[0] =
        new layerAdditionRemoval
        (
            "extrusion",
            0,
            topoChanger_,
            extrusionFaceSet_,
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
    barrelPointSet_(motionDict_.lookupOrDefault<word>("barrelPointSet","barrelPoints")),
    motionPointSet_(motionDict_.lookupOrDefault<word>("motionPointSet","motionPoints")),
    extrusionFaceSet_(motionDict_.lookupOrDefault<word>("extrusionFaceSet", "extrusionFaces")),
    initialPosition_(readScalar(motionDict_.lookup("initialPosition"))),
    initialVelocity_(readScalar(motionDict_.lookup("initialVelocity"))),
    exitVelocity_(readScalar(motionDict_.lookup("exitVelocity"))),
    curMotionVel_(curMotionVel()),
    currentPosition_(initialPosition_),
    growthLayerPosition_(0),
    tolerance_(SMALL),
    gunPoints_(0),
    motionPoints_(0),
    nOldPoints_(0)
{
    //normalize aimVector
    if ( mag(aimVector_) <= SMALL )
    {
        FatalErrorIn("gunTopoFvMesh::gunTopoFvMesh: ")
            << "Aim vector too short."
            << abort(FatalError);
    }

    aimVector_ /= (mag(aimVector_)+SMALL);

    Info<< "Initial time:" << time().value()
        << " Initial curMotionVel_:" << curMotionVel_
        << endl;

    addZonesAndModifiers();

    updateExtrudeLayerPosition();

    gunMarkup();

    motionMask_ = vertexMarkup(points());
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::gunTopoFvMesh::~gunTopoFvMesh()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::gunTopoFvMesh::update()
{
    Info << "Entering Foam::gunTopoFvMesh::update()" << endl;
    nOldPoints_ = points().size();

    // Do mesh changes (use inflation - put new points in topoChangeMap)
    autoPtr<mapPolyMesh> topoChangeMap = topoChanger_.changeMesh(true);

    // Calculate the new point positions depending on whether the
    // topological change has happened or not
    pointField newPoints;

    curMotionVel_ = curMotionVel();

    debugInfo();

    vector pointsDisplacement =
        aimVector_ * curMotionVel_*time().deltaT().value();

    if (topoChangeMap.valid())
    {
        Info<< "Topology change. Calculating motion points" << endl;

        updateGunPointLabels(topoChangeMap().reversePointMap());

        updateMappedFaces(topoChangeMap());

        if (topoChangeMap().hasMotionPoints())
        {
            Info<< "Topology change. Has premotion points" << endl;

            motionMask_ = vertexMarkup(topoChangeMap().preMotionPoints());

            // Move points inside the motionMask
            newPoints = topoChangeMap().preMotionPoints()
                      + pointsDisplacement * motionMask_;
        }
        else
        {
            Info<< "Topology change. Already set mesh points" << endl;

            motionMask_ = vertexMarkup(points());

            // Move points inside the motionMask
            newPoints = points()
                      + pointsDisplacement * motionMask_;
        }
    }
    else
    {
        Info<< "No topology change" << endl;
            motionMask_ = vertexMarkup(points());

        // Set the mesh motion
        newPoints = points()
                  + pointsDisplacement * motionMask_;
    }

    // The mesh now contains the cells with zero volume
    Info << "Executing mesh motion" << endl;
    movePoints(newPoints);
    Info << "Executed mesh motion" << endl;
    //  The mesh now has got non-zero volume cells

    //updateExtrudeLayerPosition();

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
    return initialVelocity_;// curMotionVel_;
}

// ************************************************************************* //
