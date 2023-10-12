#!/usr/bin/python
from pylab import *

# Check time step
#  >> ./checkTimeStep.py <ID> <arg>
# ID  = probe ID
# arg = A gives full time series
#     = B gives truncated time series

if (sys.argv[2] == "A" ):
    dir = "ind_probes"
elif (sys.argv[2] == "B" ):
    dir = "ind_probes_trunc"
else:
    print "Error in sys.argv[2]"
    sys.exit()

fname = dir + "/p" + str(sys.argv[1])

A = loadtxt(fname)
t = A[:,0]
Nt = len(t)

dt = zeros(Nt)
dt[0] = t[1] - t[0]
for i in range(Nt-1):
    dt[i+1] = t[i+1] - t[i]

subplot(2,1,1)
title("Time step")
plot(t,1000*dt)
ylabel("dt (ms)")
grid()

subplot(2,1,2)
plot(t,1000*dt)
xlabel("t (s)")
ylabel("dt (ms)")
axis([800,810,15,40])
grid()

savefig("figures/timeStep.png")
show()



