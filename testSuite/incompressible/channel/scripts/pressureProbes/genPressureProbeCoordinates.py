#!/usr/bin/python

from string import Template

xRange = (3.0,5,0)
zRange = (2.0,4.0)
nX     = 20
nZ     = nX
deltaX = (xRange[1] - xRange[0])/nX
deltaZ = (zRange[1] - zRange[0])/nZ

wallOffset = 0.01;

xpositions = [ xRange[0]+a*deltaX for a in range(nX) ]
zpositions = [ zRange[0]+a*deltaZ for a in range(nZ) ]

START='''
/*
 *  Probes deltaX  = {}
 *  Probes deltaZ  = {}
 */
nearWallProbes
{{
    interpolationScheme cellPoint;
    type                probes;
    enabled             true;
    writeControl        timeStep;
    writeInterval       1;
    fields              (p,U);

    probeLocations
    (
'''.format(deltaX,deltaZ)


END = '''
    );
}
'''


print START

for i,zpos in enumerate(zpositions):
    for j,xpos in enumerate(xpositions):
        print '\t({} {} {})'.format(xpos,wallOffset,zpos)

print END




