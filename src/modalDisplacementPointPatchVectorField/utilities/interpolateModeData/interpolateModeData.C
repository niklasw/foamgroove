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

Application
    writePatchPointLabels

Description
    Writes to file patch point labelLists for patches given as argument list.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "IFstream.H"
#include "volFields.H"
#include "IOobjectList.H"
#include "Tuple2.H"
#include "indexedOctree.H"
#include "treeDataCell.H"
#include <iostream>
#include <fstream>
#include <string>

#include "modeFile.H"
#include "interpolateFunctions.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::noParallel();
    argList::addOption("patch","patch","Name of patch to interpolate to.");
    argList::addOption("modeFiles","modeFiles","List of files to read mode data from.'(file1 file2 file3)'");
    argList::addOption("skipLines","skipLines","Number of lines to ignore from top of modeFile.");
    argList::addOption("delimiter","delimiter","NOT IMPLEMENTED! Mode data column delimiter. Default ','.");
    argList::addOption("coordColumn","coordColumn","Column number for first coordinate column. Default 0");
    argList::addOption("dispColumn","dispColumn","Column number for first displacements column. Default 3");
    argList::addBoolOption("rbf","Use radial basis function to interpolate");

#   include "setRootCase.H"
#   include "createTime.H"

    List<fileName> modeFiles;
    fileName modeFileName;
    label skipLines = 0;
    label coordColumn = 0;
    label dispColumn = 3;
    char delimiter = ',';

    word patchName = args.optionRead<word>("patch");
    modeFiles = args.optionReadList<fileName>("modeFiles");
    args.optionReadIfPresent("skipLines",skipLines,skipLines);
    args.optionReadIfPresent("delimiter",delimiter,delimiter);
    args.optionReadIfPresent("coordColumn",coordColumn,coordColumn);
    args.optionReadIfPresent("dispColumn",dispColumn,dispColumn);

#   include "createMesh.H"

    List<label> modeLabels(0);

    //- Initiate modeFile, so that same modeFile object
    //  is used through the loop, just change files.
    //  Thus, coordinates read only once.
    modeFile Mode(modeFiles[0], coordColumn, dispColumn, skipLines, delimiter);

    Mode.coordinatesInfo();

    forAll(modeFiles, I)
    {
        const fileName& modeFileName = modeFiles[I];
        word modeName = modeFileName.name(true);

        Mode.changeFile(modeFileName);
        Mode.displacementInfo();
        if ( args.optionFound("rbf"))
        {
            transferModeRBF( modeName, mesh, patchName, Mode);
        }
        else
        {
            transferMode( modeName, mesh, patchName, Mode, modeLabels);
        }
        Info << "Transfered " << modeFileName << " to " << nl << modeName << endl;
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
