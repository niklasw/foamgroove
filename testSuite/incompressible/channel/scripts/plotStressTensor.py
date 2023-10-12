#!/usr/bin/python
import sys,glob,os,matplotlib,re
import numpy as np
import matplotlib.pyplot as plt


font = {'family' : 'sans-serif',
        'weight' : 'normal',
        'size'   :  22}
matplotlib.rc('font', **font)

dataDirs = sys.argv[1:]
stressFilePattern = '*UPrime2Mean.xy'
velocityFilePattern = '*UMean.xy'

# Symm tensor enumeration
# SymmTensor.H:    enum components { XX, XY, XZ, YY, YZ, ZZ };
components = ['xx', 'xy', 'xz', 'yy', 'yz', 'zz']
indices    = [ 1,    2,    3,    4,    5,    6  ]
symTensorEnum= dict(zip(components,indices))

def collectData(path,pattern):
    dataFiles = glob.glob(os.path.join(path,pattern))
    T = None
    for f in dataFiles:
        if T == None:
            T = np.loadtxt(f,unpack=True)
        else:
            T+= np.loadtxt(f,unpack=True)
    T/=len(dataFiles)
    return T


def plotTensorComponent(coord,symTensor,component,xNorm=1.0,yNorm=1.0,name='',**kwargs):
    cmpI = symTensorEnum[component]
    t = symTensor[cmpI]*yNorm
    s = coord*xNorm
    plt.subplot(2,3,cmpI)
    plt.plot(s[0:len(s)/2],t[0:len(s)/2],label=name,linewidth=2,**kwargs)
    plt.grid('on')
    plt.ylabel('$\\sigma_{{{0}}}$'.format(component))

def plotMeanVelocity(coord,vector,component,xNorm=1.0,yNorm=1.0,name='',**kwargs):
    u = vector[component]*yNorm
    s = coord*xNorm
    plt.plot(s[0:len(s)/2],u[0:len(s)/2],label=name,linewidth=2,**kwargs)
    plt.grid('on')
    plt.ylabel('$\\sigma_{{{0}}}$'.format(component))



def readUTau(casePath):
    blResultsFile = os.path.join(casePath,'boundaryLayerResults.txt')
    uTau = None
    if os.path.isfile(blResultsFile):
        with open(blResultsFile) as fp:
            for line in fp:
                if 'friction velocity' in line:
                    uTau = line.split(':')[1].strip()
                    print '\tFound uTau = {} in {}'.format(uTau,blResultsFile)
                    break
    try:
        return float(uTau)
    except:
        print 'WARNING:\n\tCannot read uTau from file {}.\n\tUsing value 4.5e-2'.format(blResultsFile)
        return 4.5e-2



jet = plt.cm.jet
colors = jet(np.linspace(0, 1, len(dataDirs)))

fig1 = plt.figure(1,figsize=(18,8))
fig2 = plt.figure(2,figsize=(18,8))

for i,dataDir in enumerate(dataDirs):
    caseName = re.sub(r'.*/(subCase_[0-9]+)/.*','\\1',dataDir)
    print caseName

    uTau = readUTau(caseName)

    plt.figure(1)
    T = collectData(dataDir,stressFilePattern)
    s   = T[0]
    for cmpt in components:
        plotTensorComponent(s,T,cmpt,xNorm=1.0,yNorm=1.0/uTau**2,name=caseName,color=colors[i])

    plt.figure(2)
    U = collectData(dataDir,velocityFilePattern)
    s   = U[0]
    plotMeanVelocity(s,U,1,name=caseName,color=colors[i])

plt.figure(1)
plt.tight_layout()
plt.savefig('UMean.png')
plt.figure(2)
plt.tight_layout()
plt.legend(loc='upper right',fontsize=12)
plt.savefig('UPrime2Mean.png')
plt.show()
