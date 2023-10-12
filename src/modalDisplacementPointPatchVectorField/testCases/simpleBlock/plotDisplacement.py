#!/usr/bin/python

import re,sys
import matplotlib.pyplot as plt
from numpy import arange,asarray

log=sys.argv[1]

dt=1e-2
scaleY=1

regx=r'^.*Monitor point 0 disp.*=\s*\((.*)\).*$'

pat=re.compile(regx)

dataList = list()
with open(log,'r') as fp:
    for line in fp:
        match=pat.match(line)
        if match:
            strData = match.groups()[0]
            dataList.append(map(float,strData.split()))

dataList = asarray(dataList[::1])

Dx=dataList[:,0]
Dy=dataList[:,1]

t=arange(0,dt*(len(dataList)),dt)

plt.figure(1)
plt.plot(t,Dx,'r',t,Dy*scaleY,'g')

plt.legend(['Axial displ.','Transverse displ. * %i'%scaleY])
plt.grid(True)
plt.savefig(log+'_displacements.png')

plt.figure(4)
plt.plot(Dx,Dy*scaleY)
plt.title('Dx against Dy * %i'%scaleY)
plt.axis('equal')
plt.savefig(log+'_kringla.png')

def plotFFT(
            samples,
            sampleFrq,
            maxFrq=1e3,
            plotdata=True,
            log=False):

    import pylab,scipy
    n=len(samples)

    freq=scipy.array(range(n/2+1))/(n/2.0)
    freq=freq[1:]*sampleFrq/2.0

    if plotdata:
        Y = scipy.fft(samples)
        power = abs(Y[1:(n/2)+1])**2/n
        if log:
            pylab.loglog(freq,power)
        else:
            pylab.plot(freq,power)
        plt.xlim([0,maxFrq])

plt.figure(2)
plt.grid(True)
plt.title(log+'_FFT axial')
plotFFT(Dx,1/dt,maxFrq=40)
plt.savefig(log+'_FFT-axial.png')
plt.figure(3)
plt.grid(True)
plt.title(log+'_FFT transverse')
plotFFT(Dy,1/dt,maxFrq=40)
plt.savefig(log+'_FFT-transverse.png')
plt.show()
