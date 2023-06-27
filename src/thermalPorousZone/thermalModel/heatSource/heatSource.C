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

#include "heatSource.H"
#include "addToRunTimeSelectionTable.H"
#include "basicThermo.H"
#include "volFields.H"
#include "fvMatrices.H"
#include "fvCFD.H"
#include "zeroGradientFvPatchFields.H"
#include "DimensionedScalarField.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace porousMedia
{
    defineTypeNameAndDebug(heatSource, 0);

    addToRunTimeSelectionTable
    (
        thermalModel,
        heatSource,
        pZone
    );
}
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::porousMedia::heatSource::heatSource(const porousZone& pZone)
:
    thermalModel(pZone),
    Tmin_(readScalar(thermalCoeffs_.lookup("Tmin"))),
    Tmax_(readScalar(thermalCoeffs_.lookup("Tmax"))),
    hSource_(readScalar(thermalCoeffs_.lookup("enthalpySource"))),
    mesh_(pZone.mesh()),
    T_( mesh_.lookupObject<volScalarField>("T")),
    h_( mesh_.lookupObject<volScalarField>("h")),
    zoneVolume_(0.0)
{
    const labelList& zones = pZone_.zoneIds();
    forAll(zones, zoneI)
    {
        const labelList& cells = mesh_.cellZones()[zones[zoneI]];

        forAll(cells, cellI)
        {
            zoneVolume_ += mesh_.V()[cells[cellI]];
        }
    }
    reduce(zoneVolume_,sumOp<scalar>());

    Info << "thermalModel zone volume = "<< zoneVolume_ << " m3" << endl;
}



// * * * * * * * * * * * * * * * * Destructor    * * * * * * * * * * * * * * //

Foam::porousMedia::heatSource::~heatSource()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::porousMedia::heatSource::addEnthalpySource
(
    const basicThermo& thermo,
    const volScalarField& rho,
    fvScalarMatrix& hEqn
) const
{

    const labelList& zones = pZone_.zoneIds();
    if (zones.empty() || hSource_ == 0.0)
    {
        return;
    }

    volScalarField hSource
    (
        IOobject
        (
            "hSource",
            mesh_.time().timeName(),
            mesh_,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        mesh_,
        dimensionedScalar("hsrc",dimPower/dimVolume,0.0),
        word("zeroGradient")
    );

    forAll(zones, zoneI)
    {
        const labelList& cells = mesh_.cellZones()[zones[zoneI]];

        scalar specificHSource = hSource_/zoneVolume_;

        forAll(cells, cellI)
        {
            label curCell = cells[cellI];
            const scalar& Tcell = T_[curCell];
            hSource[curCell] = specificHSource * pos(Tcell-Tmin_) * pos(Tmax_-Tcell);
        }
    }
    Info << "Added power through enthalpy sources = "<< gSum(fvc::volumeIntegrate(hSource)) << " W" << endl;

    hEqn -= fvm::Su(hSource,h_);
}

// ************************************************************************* //
