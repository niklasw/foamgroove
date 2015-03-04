#!/usr/bin/python

import re,sys
import matplotlib.pyplot as plt
from numpy import arange,asarray,loadtxt,mean

from Strouhal import Strouhal

log=sys.argv[1]
forceLog = ''
if len(sys.argv) == 3:
    forceLog = sys.argv[2]

dt=1e-4

def plotFFT(
            samples,
            sampleFrq,
            maxFrq=1e3,
            plotdata=True,
            Log=False):

    import pylab,scipy,numpy
    n=len(samples)

    freq=scipy.array(range(n/2+1))/(n/2.0)
    freq=freq[1:]*sampleFrq/2.0

    if plotdata:
        Y = scipy.fft(samples)
        power = abs(Y[1:(n/2)+1])**2/n
        if Log:
            pylab.loglog(freq,power)
        else:
            pylab.plot(freq,numpy.log(power))
        plt.xlim([0,maxFrq])

def collectDisplacement(logFile):
    regx=r'^.*Monitor point 0 disp.*=\s*\((.*)\).*$'
    tregx = r'^Time = (.*)$'

    pat=re.compile(regx)
    tPat = re.compile(tregx)

    dataList = list()
    timeList = list()
    with open(log,'r') as fp:
        for line in fp:
            match=pat.match(line)
            tMatch = tPat.match(line)
            if tMatch:
                strData = tMatch.group(1)
                timeList.append(float(strData))
            if match:
                strData = match.groups()[0]
                dataList.append(map(float,strData.split()))

    dataList = asarray(dataList[::1])

    Dx=dataList[:,0]
    Dy=dataList[:,2]


    t = timeList[0:len(Dx)]
    dt = t[1]-t[0] # Assume constant dt (used in FFT) 

    return t,Dx,Dy

def collectForces(logFile):
    dataArray = []
    with open(logFile,'r') as fp:
        for line in fp:
            line = line.strip()
            if line[0] == '#':
                continue
            line = re.sub(r'[\(\),]',' ', line)
            dataArray.append(map(float,line.split()))
        dataArray = asarray(dataArray)

    t = dataArray[:,0]
    Fx= dataArray[:,1]
    Fy= dataArray[:,3]
    return t,Fx,Fy

t, Dx, Dy = collectDisplacement(log)

with open(log+'.displacements','w') as fp:
    for i in range(len(t)):
        fp.write('{0} {1} {2}\n'.format(t[i],Dx[i],Dy[i]))


Strouhal(0.01)
print 'WARNING: Damping above, may not corr to below results'

avgFx = 0
avgFy = 0

if forceLog:
    tf ,Fx, Fy = collectForces(forceLog)
    print 'Force averaging over last half of sample series'
    nSamples = len(Fx)
    avgFx = mean(Fx[-nSamples/2:])
    avgFy = mean(Fy[-nSamples/2:])
    plt.figure()
    plt.plot(tf,Fx,'r',tf,Fy,'g')
    plt.grid(True)
    plt.title('Average Fx = {0:0.3e}, Fy = {1:0.3e}'.format(avgFx,avgFy))
    plt.legend(['Axial p. force','Transverse p. force'])
    plt.savefig(log+'_pressureForces.png')

nSamples = len(t)

plt.figure(figsize=(18,14))
plt.subplot(221)
plt.plot(t,Dx,'r',t,Dy,'g')
avgDx = mean(Dx[-nSamples/2:])
avgDy = mean(Dy[-nSamples/2:])
plt.title('Average Dx = {0:0.3e}, Dy = {1:0.3e}'.format(avgDx,avgDy))
plt.legend(['Axial displ.','Transverse displ.'], loc='lower left')
plt.grid('on')

print 'Axial spring coefficient from avg forces and displacements:'
print '{0:0.3e}/{1:0.3e}={2:0.3e}'.format(avgFx,avgDx,avgFx/avgDx)

plt.subplot(222)
plt.plot(Dx,Dy)
plt.title('Dx against Dy')
plt.axis('equal')
plt.grid('on')

plt.subplot(223)
plt.grid(True)
plt.title('FFT axial')
plotFFT(Dx,1/dt,maxFrq=40)

plt.subplot(224)
plt.grid(True)
plt.title('FFT transverse')
plotFFT(Dy,1/dt,maxFrq=40)
plt.suptitle(log)
plt.savefig(log+'_displacements.png')
#plt.show()


