#!/usr/bin/python
from pylab import *
from matplotlib import *

# Get three individual probes from the main data file "p", and put here:
#  individual_probes/p<ID>.dat

id = [0,1,123]

A = loadtxt("p")

t = A[:,0]

B = zeros((len(t),2))
B[:,0] = t

for i in range(len(id)):
    p = A[:,id[i]+1]
    B[:,1] = p
    fname = "ind_probes/p" + str(id[i])
    print " Saving : " + fname
    savetxt(fname,B)
    
