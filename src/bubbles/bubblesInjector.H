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

Class
    bubblesInjector

Description

SourceFiles
    bubblesInjectorI.H
    bubblesInjector.C
    bubblesInjectorIO.C

\*---------------------------------------------------------------------------*/

#ifndef bubblesInjector_H
#define bubblesInjector_H

#include "Cloud.H"
#include "bubbles.H"
#include "bubblesCloud.H"
#include "IOPtrList.H"
#include "IOdictionary.H"
#include "Random.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Class forward declarations
//class bubblesCloud;

/*---------------------------------------------------------------------------*\
                           Class bubblesInjector Declaration
\*---------------------------------------------------------------------------*/

class bubblesInjector
{
    // Private data

    bubblesCloud& cloud_;
    word name_;
    dictionary dict_;
    dimensionedVector origin_;
    dimensionedScalar radius_;
    dimensionedScalar avgDiameter_;
    int nBubbles_;
    bool twoD_;

    // Private Member Functions

        //- Disallow default bitwise copy construct
        bubblesInjector(const bubblesInjector&);

        //- Disallow default bitwise assignment
        void operator=(const bubblesInjector&);


public:

    // Constructors

        bubblesInjector(bubblesCloud& bc);
        bubblesInjector(bubblesCloud& bc, Istream& is);
        
        //- Return clone
        autoPtr<bubblesInjector> clone() const
        {
            notImplemented("autoPtr<bubblesInjector> clone() const");
            return autoPtr<bubblesInjector>(NULL);
        }


    // Selectors

        //- Select null constructed
        // static autoPtr<bubblesInjector> New();

        //- Return a pointer to a new bubblesInjector created on freestore
        //  from Istream
        class iNew
        {
            bubblesCloud& bc_;
 
        public:
 
            iNew(bubblesCloud& bc)
            :
                bc_(bc)
            {}
 
            autoPtr<bubblesInjector> operator()(Istream& is) const
            {
                return autoPtr<bubblesInjector>(new bubblesInjector(bc_, is));
            }
        };

    // Member functions

        void injection();

    // Destructor

         ~bubblesInjector();

    // Operators

        // Ostream Operator
        friend Ostream& operator<<(Ostream&, const bubblesInjector&);
};

/*---------------------------------------------------------------------------*\
                           Class bubblesInjectors Declaration
\*---------------------------------------------------------------------------*/

class bubblesInjectors
:
    public IOPtrList<bubblesInjector>
{
    bubblesCloud& cloud_;
    //dictionary dict_;

public:
    
    // Constructors

        //- Construct from fvMesh and cloud
        bubblesInjectors(const fvMesh& mesh, bubblesCloud& bc);

        virtual bool readData(const fvMesh& mesh, Istream& is);

    // Member functions

        void injection()
        {
            forAll(*this, i)
            {
                operator[](i).injection();
            }
        };
    

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
