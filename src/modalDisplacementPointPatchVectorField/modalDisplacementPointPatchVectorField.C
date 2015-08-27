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

#include "modalDisplacementPointPatchVectorField.H"
#include "pointPatchFields.H"
#include "addToRunTimeSelectionTable.H"
#include "Time.H"
#include "polyMesh.H"
#include "labelIOList.H"
#include "mathematicalConstants.H"
#include "structuralModes/structuralModes.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * Private member functions  * * * * * * * * * * * //

scalar modalDisplacementPointPatchField::smoothStep
(
        const scalar s0,
        const scalar s1, 
        scalar x
) const
{
    scalar out = 1;
    if ((s1 - s0) > SMALL)
    {
        x = min(max(x,s0),s1);
        x = (x - s0)/(s1 - s0); 
        out = x*x*x*(x*(x*6 - 15) + 10);
    }
    Info << "Ramp factor = " << out << endl;
    return out;
}

void modalDisplacementPointPatchField::readPointProcAddressing()
{
    if (Pstream::parRun())
    {
        IOobject ppaHdr
        (
            "pointProcAddressing",
            meshPtr_->facesInstance(),
            meshPtr_->meshSubDir,
            *meshPtr_,
            IOobject::MUST_READ
        );

        if (!ppaHdr.headerOk())
        {
            FatalErrorIn("Foam::modalDisplacementPointPatchField::readPointProcAddressing")
                << "Cannot read pointProcAddressing file" << nl
                << "It is needed for this BC in parallel." << nl
                << ppaHdr.objectPath()<< exit(FatalError);
        }

        labelIOList pointProcAddressing(ppaHdr);

        const labelList& mp = this->patch().meshPoints();

        forAll (mp, i)
        {
            patchPointProcAddressing_[i] = pointProcAddressing[mp[i]];
        }
    }
    else
    {
        patchPointProcAddressing_ = this->patch().meshPoints();
    }
}

void modalDisplacementPointPatchField::createSerialIndexMap()
{
    /*
     * A labelList, size of the _serial_ pointPatch mapping Global mesh point
     * label to index of corresponding point in the serial patch.
     */
    labelList tmpPatchLabels(0);
    if (Pstream::master())
    {
        IOobject patchLabelsHdr
        (
            "labels_"+this->patch().name(),
            meshPtr_->time().caseConstant(),
            "structuralModes",
            *meshPtr_,
            IOobject::MUST_READ
        );

        if (!patchLabelsHdr.headerOk())
        {
            FatalErrorIn("Foam::modalDisplacementPointPatchField::createSerialIndexMap")
                << "Cannot read patchLabelList file" << nl
                << "It must be created by a serial utility before " << nl
                << "using this BC in parallel." << nl
                << patchLabelsHdr.objectPath()<< exit(FatalError);
        }

        tmpPatchLabels.append(labelIOList(patchLabelsHdr));
    }

    List<labelList> procLists(Pstream::nProcs());
    {
        procLists[Pstream::myProcNo()] = tmpPatchLabels;
        Pstream::gatherList(procLists);
        Pstream::scatterList(procLists);
    }

    const labelList& serialPatchPointLabels = procLists[0];

    globalMeshToGlobalPatchIndexMap_.resize(serialPatchPointLabels.size());

    forAll(serialPatchPointLabels,i)
    {
        const label& globalLabel = serialPatchPointLabels[i];
        globalMeshToGlobalPatchIndexMap_.insert(globalLabel,i);
    }
}

Tuple2<label,label> modalDisplacementPointPatchField::primitiveNearestPointSearch
(
    const point& p
)
{
    scalar dist = GREAT;
    label meshPointId=-1;
    forAll(this->patch().localPoints(), i)
    {
        const point& pp = this->patch().localPoints()[i];
        scalar newDist = mag(p-pp);
        if ( newDist < dist )
        {
            meshPointId = meshIndex(i);
            dist = newDist;
        }
    }

    List<scalar> procDists(Pstream::nProcs());
    List<label> procLabels(Pstream::nProcs());
    {
        procDists[Pstream::myProcNo()] = dist;
        procLabels[Pstream::myProcNo()] = meshPointId;
        Pstream::gatherList(procDists);
        Pstream::scatterList(procDists);

        Pstream::gatherList(procLabels);
        Pstream::scatterList(procLabels);
    }

    dist = GREAT;
    label procId = -1;
    label pointId = -1;
    forAll(procLabels, i)
    {
       if (procDists[i] < dist)
       {
           dist = procDists[i];
           procId = i;
           pointId = procLabels[i];
       }
    }
    //- Return tuple containing which process has closest pt
    //  and it's mesh (local) point id
    return Tuple2<label,label>(procId, pointId);
}

List<Tuple2<label,label> >
modalDisplacementPointPatchField::findMonitorPoints()
{
    IOobject monitorPointsHdr
    (
        "monitorPoints_"+this->patch().name(),
        this->dimensionedInternalField().mesh().time().caseConstant(),
        "structuralModes",
        this->db(),
        IOobject::MUST_READ
    );

    IOobject monitorLabelsHdr
    (
        "monitors_"+this->patch().name(),
        this->dimensionedInternalField().mesh().time().timeName(),
        "uniform",
        this->db(),
        IOobject::MUST_READ
    );

    List<Tuple2<label,label> > searchResult(0);

    //if (monitorLabelsHdr.headerOk())
    if (false)
    {
        labelIOList mLabels(monitorLabelsHdr);
        searchResult.setSize(mLabels.size());
        forAll(mLabels, i)
        {
           // searchResult[i]
        }
    }
    else if (monitorPointsHdr.headerOk())
    {
        IOList<point> mPoints(monitorPointsHdr);
        searchResult.setSize(mPoints.size());
        forAll (mPoints, i)
        {
            searchResult[i] = primitiveNearestPointSearch(mPoints[i]);
        }
        Info << "Found " << mPoints.size() << " monitor points." << endl;
    }
    else
    {
        Info << "Found no monitor points." << endl;
    }
    return searchResult;
}

void modalDisplacementPointPatchField::findMyMonitors()
{
    List<Tuple2<label,label> > allMonitors = findMonitorPoints();
    allMonitors_.append(allMonitors);

    forAll(allMonitors, i)
    {
        label proc = allMonitors[i].first();
        if ( Pstream::myProcNo() == proc)
        {
            myMonitors_.append
            (
                Tuple2<label,label>
                (
                    i,
                    localIndex(allMonitors[i].second())
                )
            );
        }

    }
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

modalDisplacementPointPatchField::
modalDisplacementPointPatchField
(
    const pointPatch& p,
    const DimensionedField<vector, pointMesh>& iF
)
:
    fixedValuePointPatchField<vector>(p, iF),
    rampBegin_(0),
    rampEnd_(0),
    writeDebugField_(false),
    globalMeshToGlobalPatchIndexMap_(0),
    patchPointProcAddressing_(this->size()),
    allMonitors_(0),
    myMonitors_(0)
{
    Info << "modalDisplacementPointPatchField:: In constructor 0\n" << endl;

    /*
    meshPtr_ = & this->dimensionedInternalField().mesh()();

    initIndexMaps();

    findMyMonitors();

    modesPtr_=new structuralModes(*this);
    */
}


modalDisplacementPointPatchField::
modalDisplacementPointPatchField
(
    const pointPatch& p,
    const DimensionedField<vector, pointMesh>& iF,
    const dictionary& dict
)
:
    fixedValuePointPatchField<vector>(p, iF, dict),
    rampBegin_(0),
    rampEnd_(0),
    writeDebugField_(false),
    globalMeshToGlobalPatchIndexMap_(0),
    patchPointProcAddressing_(this->size()),
    allMonitors_(0),
    myMonitors_(0)
{
    Info << "modalDisplacementPointPatchField:: In constructor 1\n" << endl;

    meshPtr_ = & this->dimensionedInternalField().mesh()();

    dict.readIfPresent("rampBegin", rampBegin_);
    dict.readIfPresent("rampEnd", rampEnd_);

    dict.readIfPresent("writeDebugField",writeDebugField_);

    initIndexMaps();

    findMyMonitors();

    modesPtr_ = new structuralModes(*this);

    if (!dict.found("value"))
    {
        updateCoeffs();
    }
}


modalDisplacementPointPatchField::
modalDisplacementPointPatchField
(
    const modalDisplacementPointPatchField& ptf,
    const pointPatch& p,
    const DimensionedField<vector, pointMesh>& iF,
    const pointPatchFieldMapper& mapper
)
:
    fixedValuePointPatchField<vector>(ptf, p, iF, mapper),
    rampBegin_(ptf.rampBegin_),
    rampEnd_(ptf.rampEnd_),
    writeDebugField_(false),
    globalMeshToGlobalPatchIndexMap_(0),
    patchPointProcAddressing_(this->size()),
    allMonitors_(0),
    myMonitors_(0)
{
    Info << "modalDisplacementPointPatchField:: In constructor 2" << endl;
}


modalDisplacementPointPatchField::
modalDisplacementPointPatchField
(
    const modalDisplacementPointPatchField& ptf,
    const DimensionedField<vector, pointMesh>& iF
)
:
    fixedValuePointPatchField<vector>(ptf, iF),
    rampBegin_(ptf.rampBegin_),
    rampEnd_(ptf.rampEnd_),
    writeDebugField_(false),
    globalMeshToGlobalPatchIndexMap_(0),
    patchPointProcAddressing_(this->size()),
    allMonitors_(0),
    myMonitors_(0)
{
    Info << "modalDisplacementPointPatchField:: In constructor 3," << endl;

    meshPtr_ = & this->dimensionedInternalField().mesh()();

    initIndexMaps();

    findMyMonitors();

    modesPtr_=new structuralModes(*this);
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void modalDisplacementPointPatchField::updateCoeffs()
{
    clock timer;

    if (this->updated())
    {
        return;
    }

    vectorField pCoords = this->patch().localPoints();

    const label tIndex = meshPtr_->time().timeIndex();

    if (writeDebugField_)
    {
        //- Write the raw displacement from mode input
        Field<vector>::operator=(modesPtr_->modeDisplacement());
    }
    else if (tIndex > rampBegin_)
    {
        const volScalarField& p = meshPtr_->lookupObject<volScalarField>("p");
        Field<vector>::operator=
        (
            modesPtr_->calculatedModeDisplacement
            (
                 smoothStep(rampBegin_, rampEnd_, tIndex) * p
            )
        );
    }

    fixedValuePointPatchField<vector>::updateCoeffs();

    forAll (myMonitors_,i)
    {
        label monitorIndex = myMonitors_[i].first();
        label localLabel = myMonitors_[i].second();

        const vector& mDisp = this->operator[](localLabel);
        Pout << "Monitor point " << monitorIndex
             << " displacement = " << mDisp  << endl;
    }
    Info << "Updated displacement field patch " << this->patch().name()
         << " in " << timer.elapsedClockTime() <<" s." << endl;
}

void modalDisplacementPointPatchField::writeMonitors() const
{
    IOobject monitorsHdr
    (
        "monitors_"+this->patch().name(),
        this->dimensionedInternalField().mesh().time().timeName(),
        "uniform",
        this->db()
    );

    List<label> monitorLabels(0);
    forAll(allMonitors_, i)
    {
        monitorLabels.append(allMonitors_[i].second());
    }

    labelIOList monitorData(monitorsHdr, monitorLabels);

    monitorData.write();
}

void modalDisplacementPointPatchField::write(Ostream& os) const
{
    pointPatchField<vector>::write(os);
    os.writeKeyword("rampBegin") << rampBegin_ << token::END_STATEMENT << nl;
    os.writeKeyword("rampEnd") << rampEnd_ << token::END_STATEMENT << nl;
    writeEntry("value", os);

    writeMonitors();

    modesPtr_->writeODEData();
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePointPatchTypeField
(
    pointPatchVectorField,
    modalDisplacementPointPatchField
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
