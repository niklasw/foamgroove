#!/usr/bin/python

from string import Template

startPos = 0.1
endPos = 5.9;
nSets = 28;
stepSize = (endPos-startPos)/float(nSets)
xpositions = [ startPos+a*stepSize for a in range(nSets) ]
zpositions = [ startPos+a*stepSize for a in range(nSets) ]

START='''
/*--------------------------------*- C++ -*----------------------------------*\
| =========                 |                                                 |
| \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox           |
|  \\    /   O peration     | Version:  2.3.0                                 |
|   \\  /    A nd           | Web:      www.OpenFOAM.org                      |
|    \\/     M anipulation  |                                                 |
\*---------------------------------------------------------------------------*/
FoamFile
{
    version     2.0;
    format      ascii;
    class       dictionary;
    location    "system";
    object      sampleDict;
}
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

interpolationScheme cellPoint;

setFormat       raw;

type    sets;

line
{
    type    uniform;
    axis    y;
    nPoints 100;
};

sets
(
'''

TEMPLATE='''
    l$number
    {
        $$line
        start   ( 2 0 $zposition );
        end     ( 2 2 $zposition );
    }
    '''

END = '''
);

fields          ( UMean UPrime2Mean );

// ************************************************************************* //

'''


print START

for i,zpos in enumerate(zpositions):
    tpl = Template(TEMPLATE)
    n='{0:02d}'.format(i)
    curSet = tpl.safe_substitute(number=i,zposition=zpos)
    print curSet

print END




