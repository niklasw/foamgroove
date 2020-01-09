#!/usr/bin/env python3

from jinja2 import Template
import sys
from numpy import linspace,arange


funcStr='''
    type            surfaces;
    functionObjectLibs ("libsampling.so");
    writeControl   writeTime;
    writeInterval  1;

    surfaceFormat   vtk;
    fields          ( T U );

    interpolationScheme cellPoint;

    surfaces
    (
        {%for coord in coords%}
        {{name}}_{{loop.index0}}
        {
            type            cuttingPlane;
            planeType       pointAndNormal;
            pointAndNormalDict
            {
                basePoint       ({%for c in coord%}{{c}} {%endfor%});
                normalVector    ({%for v in normal%}{{v}} {%endfor%});
            }
            interpolate     false;
        }
        {%endfor%}
    );
'''

T = Template(funcStr)

origin  = [80,50,5]
X = list(arange(58.3,112,2))
Y = list(arange(29.9,61,1))
Z = list(arange(0.2,7,0.5))

xNormal = (1,0,0)
yNormal = (0,1,0)
zNormal = (0,0,1)
xCoords = [[val,origin[1],origin[2]] for val in X]
yCoords = [[origin[0],val,origin[2]] for val in Y]
zCoords = [[origin[0],origin[1],val] for val in Z]

selector = {'x':[xNormal,xCoords],
            'y':[yNormal,yCoords],
            'z':[zNormal,zCoords]}

Coords=[]
Normal=[]
if len(sys.argv) == 2:
    direction = sys.argv[1]
    Normal = selector[direction][0]
    Coords = selector[direction][1]
else:
    print('Usage: $0 <x|y|z>')
    sys.exit(0)


out = T.render(coords=Coords, normal=Normal, name=direction)
print(out)

