#!/usr/bin/python
from pylab import *

# Calculations involving standard deviation of pressure in
# a selected probe. Use the truncated time interval.
#  >> ./kraichnan.py <ID>

V0 = 1.15 # Guess of typical mean center velocity
beta = 3e-3

fname = "ind_probes_trunc/p" + str(sys.argv[1])
print " File : ",fname

A = loadtxt(fname)
t = A[:,0]
p = A[:,1]
sp = std(p)
alpha = 2*sp/(beta*V0**2) 

print "std(p) = ",sp
print "alpha  = ",alpha

