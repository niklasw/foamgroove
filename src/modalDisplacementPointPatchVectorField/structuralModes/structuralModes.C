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

\*---------------------------------------------------------------------------*/

#include "structuralModes.H"
#include "polyMesh.H"
#include "OFstream.H"
#include "scalarListIOList.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTemplateTypeNameAndDebug(IOPtrList<structuralMode>, 0);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::structuralModes::structuralModes(const modalDisplacementPointPatchField& BC)
:
    IOPtrList<structuralMode>
    (
        IOobject
        (
            "modeData_"+BC.patch().name(),
            BC.db().time().caseConstant(),
            "structuralModes",
            BC.db(),
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        ),
        structuralMode::iNew(BC)
    ),
    BC_(BC),
    patchPoints_(BC.patch().localPoints()),
    fluidDensity_(dimensionedScalar("fluidDensity",dimMass/dimVol,1.0))

{
    IOdictionary modesDict
    (
        IOobject
        (
            "modesDict",
            BC.db().time().caseConstant(),
            "structuralModes",
            BC.db(),
            IOobject::MUST_READ
        )
    );
    fluidDensity_ = modesDict.lookupOrDefault
                    (
                        "fluidDensity",
                        dimensionedScalar("fluidDensity",dimMass/dimVol,1.0)
                    );

    odeSolver_ = modesDict.lookupOrDefault<word>
                 (
                    "odeSolver",
                    "Newmark"
                 );

    Info << "Selected ode solver " << odeSolver_ << endl;

    readODEData();
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //


Foam::vectorField Foam::structuralModes::modeDisplacement() const
{
    vectorField sumDisplacements(BC_.size(),vector::zero);
    forAll(*this, i)
    {
        sumDisplacements += operator[](i).modeDisplacement();
    }
    return sumDisplacements;
}

Foam::vectorField Foam::structuralModes::calculatedModeDisplacement
(
    const volScalarField& p
) const
{
    dimensionedScalar rho = fluidDensity();
    if (p.dimensions() == dimVol/dimMass)
    {
        rho.value() = 1.0;
    }

    vectorField sumDisplacements(BC_.size(),vector::zero);
    forAll(*this, i)
    {
        //- Because operator[](i) returns a const reference,
        //  we need to cast off the const to be able to use
        //  the non-const motionSolver
        structuralMode& mode = const_cast<structuralMode&>(operator[](i));
        scalar coeff = mode.solveMotionEquation(rho*p, odeSolver_);
        sumDisplacements += mode.modeDisplacement()*coeff;
    }
    return sumDisplacements;
}

void Foam::structuralModes::writeODEData() const
{
    IOobject ODEDataHdr
    (
        "ODEData_"+BC_.patch().name(),
        BC_.dimensionedInternalField().mesh().time().timeName(),
        "uniform",
        BC_.db()
    );

    List<scalarList> odeData(this->size());
    forAll(*this, i)
    {
        odeData[i] = operator[](i).odeData();
    }
    scalarListIOList OdeData(ODEDataHdr, odeData);
    OdeData.write();
}

void Foam::structuralModes::readODEData()
{
    IOobject ODEDataHdr
    (
        "ODEData_"+BC_.patch().name(),
        BC_.dimensionedInternalField().mesh().time().timeName(),
        "uniform",
        BC_.db(),
        IOobject::MUST_READ
    );

    if (ODEDataHdr.headerOk())
    {
        scalarListIOList OdeData(ODEDataHdr);

        if (OdeData.size() == this->size())
        {
            forAll(*this, i)
            {
                Info << "ODEData found for mode "<< operator[](i).name()
                     << ": " << OdeData[i] << nl << endl;
                operator[](i).odeData() = OdeData[i];
            }
        }
        else
        {
            FatalErrorIn("Foam::structuralModes::readODEData()")
                << "List size in \n" << ODEDataHdr.path() << nl
                << "does not match number of modes." << abort(FatalError);
        }
    }
}

bool Foam::structuralModes::readData(Istream& is)
{
    PtrList<structuralMode>::read(is, structuralMode::iNew(BC_));
    return is.good();
}

// ************************************************************************* //
