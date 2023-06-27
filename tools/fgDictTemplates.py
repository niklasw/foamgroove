from string import Template

class blockMesh(Template):
    plate ="""
/*--------------------------------*- C++ -*----------------------------------*\\
|==========                 |                                                 |
| \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox           |
|  \\    /   O peration     | Version:  2.0.0                                 |
|   \\  /    A nd           | Web:      www.OpenFOAM.org                      |
|    \\/     M anipulation  |                                                 |
\*---------------------------------------------------------------------------*/
FoamFile
{
    version     2.0;
    format      ascii;
    class       dictionary;
    object      blockMeshDict;
}
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

convertToMeters $scale;

vertices
(
$vertices
);

blocks
(
    hex (0 1 2 3 4 5 6 7) $resolution simpleGrading (1 1 1)
);

edges ();
boundary
(
    slipSides
    {
        type wall;
        faces
        (
            (3 7 6 2)
            (1 5 4 0)
        );
    }
    inlet
    {
        type patch;
        faces
        (
            (0 4 7 3)
        );
    }
    outlet
    {
        type patch;
        faces
        (
            (2 6 5 1)
        );
    }
    lowerWall
    {
        type wall;
        faces
        (
            (0 3 2 1)
        );
    }
    slipRoof
    {
        type wall;
        faces
        (
            (4 5 6 7)
        );
    }
);

mergePatchPairs ();

"""


    def __init__(self):
        Template.__init__(self,self.plate)
        self.items =['vertices','resolution']

class snappyHexMesh(Template):
    plate="""FoamFile
{
    version     2.0;
    format      ascii;
    class       dictionary;
    object      autoHexMeshDict;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

castellatedMesh true;
snap            true;
addLayers       true;


geometry
{
    /*
    box
    {
        type searchableBox;
        min (1 1 1);
        max (2 2 2);
    }

    cylinder
    {
        type searchableCylinder;
        point1  (0 -0.5 0);
        point2  (0  0.5 0);
        radius 0.1;
    }

    example.stl
    {
        type triSurfaceMesh;
        regions
        {
            secondSolid
            {
                name mySecondPatch;
            }
        }
    }
    cylinderExample.stl
    {
        type triSurfaceMesh;
    }
    */

$geometry
};



castellatedMeshControls
{
    maxLocalCells 3000000;

    maxGlobalCells 20000000;

    minRefinementCells 0;

    maxLoadUnbalance 0.10;

    nCellsBetweenLevels 2;

    features
    (
    /*
        {
            file "$featuresFile";
            level 2;
            // Example levels ((0 2) (0.3 1));
        }
    */
    );

    refinementSurfaces
    {
        /* example try to create zones
        heater
        {
            level (1 1);
            faceZone heater;
            cellZone heater;
            cellZoneInside inside;
        }

        example.stl
        {
            level (2 2);
            regions
            {
                secondSolid {level (3 4);}
            }
            patchInfo
            {
                type wall;
                inGroups (wallGroup);
            }
        }
        cylinder.stl
        {
            level (2 2);
            faceZone            MRF;
            cellZone            MRF;
            cellZoneInside      inside;
            faceType            internal; //baffle,boundary
        }
        */

$refinementSurfaces

    }

    resolveFeatureAngle 30;

    refinementRegions
    {
        /*
        box
        {
            mode inside;
            levels ((1.0 4));
        }

        sphere.stl
        {
            mode distance;
            levels ((1.0 5) (2.0 3));
        }
        */
    }

    locationInMesh ( 0 0 0 );

    allowFreeStandingZoneFaces true;
}



snapControls
{
    nSmoothPatch 3;

    tolerance 2.0;

    nSolveIter 30;

    nRelaxIter 5;

    nFeatureSnapIter 10;

    implicitFeatureSnap true;

    explicitFeatureSnap false;

    multiRegionFeatureSnap false;
}



addLayersControls
{
    relativeSizes true;

    // Layer thickness specification. This can be specified in one of four ways
    // - expansionRatio and finalLayerThickness (cell nearest internal mesh)
    // - expansionRatio and firstLayerThickness (cell on surface)
    // - overall thickness and firstLayerThickness
    // - overall thickness and finalLayerThickness

    expansionRatio 1.2;
    finalLayerThickness 0.6;
        //firstLayerThickness 0.1;
        //thickness 0.5;

    minThickness 0.25;

    layers
    {
        wallGroup
        {
            nSurfaceLayers 4;
        }
    }


    nGrow 1;

    // Advanced settings

    featureAngle 60;

    slipFeatureAngle 30;

    nRelaxIter 5;

    nSmoothSurfaceNormals 1;

    nSmoothNormals 3;

    nSmoothThickness 10;

    maxFaceThicknessRatio 0.5;

    maxThicknessToMedialRatio 0.3;

    nMedialAxisIter 10;

    minMedianAxisAngle 150; //90;

    nBufferCellsNoExtrude 0;

    nLayerIter 50;

    nRelaxedIter 20;

    // Optional: limit the number of steps walking away from the surface.
    // Default is unlimited.
    //nMedialAxisIter 10;

    // Optional: smooth displacement after medial axis determination.
    // default is 0.
    //nSmoothDisplacement 90;

    // (wip)Optional: do not extrude a point if none of the surrounding points is
    // not extruded. Default is false.
    //detectExtrusionIsland true;
}



meshQualityControls
{
    maxNonOrtho 70;

    maxBoundarySkewness 20;

    maxInternalSkewness 4;

    maxConcave 80;

    minVol 1e-30;

    minTetQuality -1e30; //1e-9;

    minArea -1;

    minTwist 0.05;

    minDeterminant 0.001;

    minFaceWeight 0.05;

    minVolRatio 0.01;

    minTriangleTwist -1;

    // Advanced

    nSmoothScale 4;

    errorReduction 0.75;

    relaxed
    {
        maxNonOrtho 80;
    }

}


// Advanced

debug 0;

mergeTolerance 1E-6;


// ************************************************************************* //
"""

    def __init__(self):
        Template.__init__(self,self.plate)
        self.items =['geometry','featuresFile','refinementSurfaces']


class snappyHexMesh21(Template):
    plate="""FoamFile
{
    version     2.0;
    format      ascii;
    class       dictionary;
    object      autoHexMeshDict;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

castellatedMesh true;
snap            true;
addLayers       false;


geometry
{
    /*
    box
    {
        type searchableBox;
        min (1 1 1);
        max (2 2 2);
    }
    */

$geometry
};



castellatedMeshControls
{
    maxLocalCells 3000000;

    maxGlobalCells 20000000;

    minRefinementCells 0;

    maxLoadUnbalance 0.10;

    nCellsBetweenLevels 2;

    features
    (
        /*
        {
            file "$featuresFile";
            level 0;
        }
        */
    );

    refinementSurfaces
    {
        /* example try to create zones
        heater
        {
            level (1 1);
            faceZone heater;
            cellZone heater;
            cellZoneInside inside;
        }
        */

$refinementSurfaces
    }

    resolveFeatureAngle 30;

    refinementRegions
    {
        /*
        box
        {
            mode inside;
            levels ((1.0 4));
        }
        */
    }

    locationInMesh ( 0 0 0 );

    allowFreeStandingZoneFaces false;
}



snapControls
{
    nSmoothPatch 3;

    tolerance 4.0;

    nSolveIter 30;

    nRelaxIter 8;

    nFeatureSnapIter 20;
}



addLayersControls
{
    relativeSizes true;

    layers
    {
        ".*[Ww]all.*"
        {
            nSurfaceLayers 3;
        }

    }

    expansionRatio 1.2;

    finalLayerThickness 0.3;

    minThickness 0.25;

    nGrow 1;

    // Advanced settings

    featureAngle 60;

    nRelaxIter 5;

    nSmoothSurfaceNormals 1;

    nSmoothNormals 3;

    nSmoothThickness 10;

    maxFaceThicknessRatio 0.5;

    maxThicknessToMedialRatio 0.3;

    minMedianAxisAngle 178;

    nBufferCellsNoExtrude 0;

    nLayerIter 50;

    nRelaxedIter 20;
}



meshQualityControls
{
    maxNonOrtho 70;

    maxBoundarySkewness 20;

    maxInternalSkewness 4;

    maxConcave 150;

    minVol 1e-30;

    minTetQuality 1e-30;

    minArea -1;

    minTwist 0.02;

    minDeterminant 0.001;

    minFaceWeight 0.05;

    minVolRatio 0.01;

    minTriangleTwist -1;

    // Advanced

    nSmoothScale 4;

    errorReduction 0.75;

    relaxed
    {
        maxNonOrtho 75;
    }

}


// Advanced

debug 0;

mergeTolerance 1E-6;


// ************************************************************************* //
"""

    def __init__(self):
        Template.__init__(self,self.plate)
        self.items =['geometry','featuresFile','refinementSurfaces']

class dbDict:
    def __init__(self):
        self.tab = '    '
        self.nl = '\n'
        self.nlc = ';\n'
        self.start = '{\n'
        self.end = '}\n'
        self.level = 0
        self.indent = self.level*self.tab
        self.entries=[]
        self.string = ''

    def ilevel(self,i):
        self.level += i
        self.indent = self.level*self.tab

    def startDict(self,name):
        s = self.indent+name+self.nl
        s += self.indent+self.start
        self.ilevel(1)
        return s

    def endDict(self):
        self.ilevel(-1)
        s = self.indent+self.end
        return s

    def entry(self,name,value):
        return self.indent+name+' '+value+self.nlc

    def new(self,name,level=0,subDict=False):
        self.ilevel(level)
        if not subDict:
            self.string = self.startDict(name)
        else:
            self.string += self.startDict(name)

    def addEntry(self,key='',value=''):
        #self.ilevel(1)
        self.string += self.entry(key,value)
        #self.ilevel(-1)

    def finish(self,level=0):
        self.string+= self.endDict()
        self.ilevel(-1*level)

    def flush(self):
        out = self.string
        self.string = ''
        return out

    def subDict(self, name='', key='', value=''):
        self.new(name)
        self.addEntry(key,value)
        self.finish()
        return self.flush()

