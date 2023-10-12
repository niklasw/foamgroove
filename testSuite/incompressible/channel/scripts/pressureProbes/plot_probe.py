#!/usr/bin/python
from pylab import *

# Plot the selected probe, whole time interval

fname = "ind_probes/p" + str(sys.argv[1])
print " File : ",fname

A = loadtxt(fname)
t = A[:,0]
p = A[:,1]

plot(t,p)
grid()

show()
