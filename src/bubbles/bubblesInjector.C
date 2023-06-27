/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 1991-2007 OpenCFD Ltd.
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

\*---------------------------------------------------------------------------*/

#include "bubblesInjector.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

namespace Foam
{
        defineTemplateTypeNameAndDebug(IOPtrList<bubblesInjector>, 0);
}


Foam::bubblesInjectors::bubblesInjectors(const fvMesh& mesh, bubblesCloud& bc)
:
    IOPtrList<bubblesInjector>
    (
        IOobject
        (
            "bubblesInjectors",
            mesh.time().constant(),
            mesh,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        ),
        bubblesInjector::iNew(bc)
    ),
    cloud_(bc)
{}

//Foam::bubblesInjector::bubblesInjector(Cloud<bubbles>& bc, Istream& is)
Foam::bubblesInjector::bubblesInjector(bubblesCloud& bc, Istream& is)
:
    cloud_(bc),
    name_(is),
    dict_(is),
    origin_( dict_.lookup("origin")),
    radius_( dict_.lookup("radius")),
    avgDiameter_(dict_.lookup("avgDiameter")),
    nBubbles_( 0 ),
    twoD_( false )
{
    dict_.lookup("nBubbles") >> nBubbles_;
    dict_.lookupOrDefault("twoD", false) >> twoD_;
    injection();
}

void Foam::bubblesInjector::injection()
{
    Random random(label(0));

    scalar pRadius(0);
    vector pos=vector::zero;

    const volVectorField& U = cloud_.pMesh().lookupObject<const volVectorField>("U");
    //const volScalarField& p = cloud_.pMesh().lookupObject<const volScalarField>("p");

    label counter = 0;
    for ( int i=0; i<nBubbles_; i++)
    {
        pRadius=radius_.value()*random.scalar01();
        pos = random.vector01();
        if ( twoD_ )
        {
            pos.z() = 0.0;
        }
        pos = pRadius*pos/(SMALL+mag(pos))+origin_.value();
        // Ugly here: diameter "variance" is welded...
        scalar d=avgDiameter_.value()+avgDiameter_.value()/3.0*random.GaussNormal();
        scalar m = d*cloud_.rhop();
        label celli = cloud_.pMesh().findCell(pos);

        if ( celli >= 0 )
        {
            bubbles* bPtr = new bubbles(cloud_,pos,celli,m,U[celli]);
            cloud_.addParticle(bPtr);
            counter += 1;
        }
    }
    Info << "Injecting " << counter << " particles." << endl;
}

bool Foam::bubblesInjectors::readData(const fvMesh& mesh, Istream &is)
{
    clear();

    IOPtrList<bubblesInjector> newBubbleInjectors
    (
        IOobject
        (
            "bubblesInjectors",
            mesh.time().constant(),
            mesh,
            IOobject::MUST_READ,
            IOobject::NO_WRITE,
            false               // Don't register new injectors with objectRegistry
        ),
        bubblesInjector::iNew(cloud_)
    );

    transfer(newBubbleInjectors);

    return is.good();
}


// * * * * * * * * * * * * * * * * Selectors * * * * * * * * * * * * * * * * //

/*
Foam::autoPtr<Foam::bubblesInjector> Foam::bubblesInjector::New()
{
    return autoPtr<bubblesInjector>(new bubblesInjector);
}
*/

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::bubblesInjector::~bubblesInjector()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::Ostream& Foam::operator<<(Ostream& os, const bubblesInjector& injector)
{
        return os;
}

// ************************************************************************* //
