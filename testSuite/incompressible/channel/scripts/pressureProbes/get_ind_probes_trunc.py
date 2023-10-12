#!/usr/bin/python
from pylab import *
from matplotlib import *

# Get three individual probes from the main data file "p".
# Truncate to an interval unaffected by the trip, and put here:
#  ind_probes_trunc/p<ID>.dat

tStart = 200 # The trip is active until t=100s
id = [70,71,72,73,74,75,76,77,78,79,80]

A = loadtxt("p")

t = A[:,0]
Nt = len(t)
print " Total number of time steps = ", Nt
ind = 0
while( t[ind] < tStart ):
    ind = ind + 1
print " Index start of truncated time interval  : ", ind
print " First sample of truncated time interval : ", t[ind] 

B = zeros((Nt-ind-1,2))
B[:,0] = t[ind:-1]

for i in range(len(id)):
    p = A[ind:-1,id[i]+1]
    B[:,1] = p
    fname = "ind_probes_trunc/p" + str(id[i])
    print " Saving : " + fname
    savetxt(fname,B)
