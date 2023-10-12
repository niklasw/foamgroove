#!/usr/bin/python
from pylab import *
from spectralFcns import *
from filter import *

# Calculations involving the spectral density.
# Use the truncated time interval.
#  >> ./spectrum.py <ID>

V0 = 1.15
delta = 1.0
f0 = V0/delta
q0 = V0**2/2.0

print " f0 = ",f0," Hz"
print " q0 = ",q0," kg/ms2"

fname = "ind_probes_trunc/p" + str(sys.argv[1])
print " File : ",fname

A = loadtxt(fname)
t = A[:,0]
p = A[:,1]

Nt = len(t)
T = t[-1] - t[0]

(f,psd) = sd_fcn(p,T)
psd_B = filter(psd,20)

F = psd*f0/(q0**2)

FdB = 10*log10(F)

Fwadmark = wadmark(f/f0)

semilogx(f/f0,FdB,label=fname)
semilogx(f/f0,Fwadmark,"k",linewidth=2.0,label="Wadmark (1967)")
axis([0.03,7,-100,-30])
grid()
xlabel("f/f0")
ylabel("PSD (dB)")
legend(loc=3)

savefig("figures/spectrum.png")
show()
