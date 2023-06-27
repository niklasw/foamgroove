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

Application
    dictionaryTest

Description

http://www.cfd-online.com/Forums/openfoam/81315-writing-subdict-dictionary.html

\*---------------------------------------------------------------------------*/

#include "argList.H"
#include "fvCFD.H"
#include "IOstreams.H"
#include "IOobject.H"
#include "IFstream.H"
#include "OFstream.H"
#include "dictionary.H"
#include "stringOps.H"

using namespace Foam;


namespace Foam
{

    template<typename T>
    void setUniformValue(dictionary& dict, const word& name, const T& value)
    {
        OStringStream os;
        os << "uniform" << " " << value;
        dict.set(name,word(os.str(),false));
    }

    IOobject checkHeader
    (
        const word& name,
        const fileName& path,
        const Time& runTime
    )
    {
        IOobject dictHdr
        (
            "fvSchemes",
            runTime.system(),
            runTime,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        );
        if (dictHdr.headerOk())
        {
            return dictHdr;
        }
        else
        {
            FatalErrorIn("Foam::checkHeader()")
                << "Cannot open dict. " << name << exit(FatalError);
        }
    }

    void updateFvSchemes(const Time& runTime)
    {
        IOdictionary dict
        (
            checkHeader("fvSchemes",runTime.system(),runTime)
        );
        dictionary& divSchemes = dict.subDict("divSchemes");
    }

    void updateFvSolution()
    {
        IOdictionary dict
        (
            checkHeader("fvSolution",runTime.system(),runTime)
        );

        const dictionary& pisoDict = dict.subDict("PISO");
        const label nCorr  = pisoDict.lookupOrDefault<label>("nCorrectors",3);
        const label nnCorr = pisoDict.lookupOrDefault<label>("nNonOrthogonalCorrectors",1);


    }

    void updateTurbulenceProperties()
    {

    }

    void updateLESProperties()
    {

    }

    void setTurbulence(dictionary& dict)
    {
        scalar Cmu = 0.09;

        dictionary& turbulence = dict.subDict("turbulence");
        dictionary& velocity   = dict.subDict("velocity");

        scalar mixingLength = readScalar(turbulence.lookup("mixingLength"));
        scalar intensity    = readScalar(turbulence.lookup("intensity"));
        scalar referenceU   = readScalar(velocity.lookup("reference"));

        scalar k      = 1.5*pow(intensity*referenceU,2);
        scalar eps    = pow(Cmu,0.75)*pow(k,1.5)/mixingLength;
        scalar omega  = eps/(Cmu*k);
        //scalar nut    = Cmu*pow(k,2)/eps;
        scalar nuTilda= k/(omega*pow(Cmu,0.25));

        setUniformValue(turbulence, "initialK", k);
        setUniformValue(turbulence, "initialEpsilon", eps);
        setUniformValue(turbulence, "initialOmega", omega);
        setUniformValue(turbulence, "initialnuTilda", nuTilda);
    }
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
//  Main program:

int main(int argc, char *argv[])
{
    argList::noParallel();

    #include "setRootCase.H"
    #include "createTime.H"

    Info << "Case updated." << endl;
    Info << "\nEnd" << endl;
    return 0;
}


// ************************************************************************* //
