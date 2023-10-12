#!/usr/bin/python
from pylab import *
from filter import *

p = linspace(1,4,4)
pf = filter(p,4)

print p
print pf
