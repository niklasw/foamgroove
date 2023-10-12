#!/usr/bin/python
from pylab import *

A = loadtxt("p")

Np = len(A[0,:])

t = A[:,0]
p1 = A[:,1]
p2 = A[:,Np-1]

plot(t,p1)
plot(t,p2)

show()
