#!/usr/bin/python
from pylab import *

# Plot some probes

id = [0,1,123]

for i in range(len(id)):
    fname1 = "ind_probes/p" + str(id[i])
    fname2 = "ind_probes_trunc/p" + str(id[i])
    A1 = loadtxt(fname1)
    A2 = loadtxt(fname2)
    t1 = A1[:,0]
    t2 = A2[:,0]
    p1 = A1[:,1]
    p2 = A2[:,1]
    subplot(2,1,1), plot(t1,p1,label=fname1)
    subplot(2,1,2), plot(t2,p2,label=fname2)

subplot(2,1,1)
legend()
grid()

subplot(2,1,2)
legend()
grid()

show()
    
