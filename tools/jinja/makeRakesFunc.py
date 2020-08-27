#!/usr/bin/env python3

from jinja2 import Template
import sys
from numpy import linspace,arange


funcStr='''
    type            sets;
    setFormat       raw;
    interpolationScheme cellPoint

    functionObjectLibs ("libsampling.so");

    writeControl   writeTime;
    writeInterval  100;

    fields          ( U k nut epsilon );


    sets
    (
        {%for start_p in start_points%}
        {% set end_p = end_points[loop.index0] %}
        {{name}}_{{loop.index0}}
        {
            type    uniform;
            nPoints 100;
            axis    distance;
            start   ({%for c in start_p%}{{c}} {%endfor%});
            end     ({%for c in end_p%}{{c}} {%endfor%});
        }
        {%endfor%}
    );
'''

T = Template(funcStr)

x0 = -1500
y0 = -1500
z0 = 0
x1 = 1500
y1 = -1500
z1 = 300

lines_name = 'rake'
start_points = [(x,y0,z0) for x in range(x0,x1,200)]
end_points   = [(x,y1,z1) for x in range(x0,x1,200)]

out = T.render(start_points=start_points, end_points=end_points, name=lines_name)
print(out)

