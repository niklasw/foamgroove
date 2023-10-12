#!/usr/bin/python

import sys,math,os
from glob import glob
from subprocess import Popen,PIPE

def readNu(casePath):
    transportProperties = os.path.join(casePath,'constant','transportProperties')
    nu = None
    if os.path.isfile(transportProperties):
        with open(transportProperties) as fp:
            for line in fp:
                if 'nu' in line:
                    nu = line.split()[-1].strip().rstrip(';')
    try:
        return float(nu)
    except:
        print 'WARNING:\n\tCannot read nu from file {}.\n\tUsing value 5.0e-5'.format(transportProperties)
        return 0.0


class boundaryLayer:
    def __init__(self,bulkVelocity,thickness,kinematicViscosity, density=1):
        self.U0 = bulkVelocity
        self.delta = thickness
        self.nu = kinematicViscosity
        self.rho = density
        self.wallShearStress = 0

    def setWallShearStress(self,tau):
        self.wallShearStress = abs(tau)

    def ReTau(self):
        return math.sqrt(self.wallShearStress/self.rho)*self.delta/self.nu

    def frictionVelocity(self):
        return self.ReTau()*self.nu/self.delta

    def yPlus(self,y=0.015):
        return y*self.frictionVelocity()/self.nu

    def __str__(self):
        s   = '\nBoundary layer data:'
        s +=  '\nAverage wall shear stress        = {0:10.3e}'.format(self.wallShearStress)
        s +=  '\nReynolds number Re_tau           = {0:10.3e}'.format(self.ReTau())
        s +=  '\nAverage friction velocity        = {0:10.3e}'.format(self.frictionVelocity())
        s +=  '\nAverage yPlys                    = {0:10.3e}'.format(self.yPlus())
        s +=  '\n'
        return s

    def csvHead(self):
        s="{},{},{},{}".format('wallShearStress','ReTau','frictionVelocity','yPlus')
        return s
    def csv(self):
        s="{},{},{},{}".format(self.wallShearStress,self.ReTau(),self.frictionVelocity(),self.yPlus())
        return s

    def info(self):
        print self

def runFoamCommand(cmd,case):
    cmd.extend(['-case',case])
    P = Popen(cmd,stdout=PIPE,stderr=PIPE)
    stdout,stderr = P.communicate()
    return stdout


def patchAverageCmd(fieldName,patchName):
    cmd = ['postProcess', \
           '-func', \
           'patchAverage(name={0},{1})'.format(patchName,fieldName),
           '-latestTime']
    return cmd

def volumeAverageCmd(fieldName):
    cmd = [ 'pimpleFoam', '-postProcess', \
            '-func', \
            'volumeAverage(fields=(delta))', \
            '-latestTime']
    return cmd

def patchAverageRegexp(fieldName,patchName):
    return '^\s*average\({}\) of {} = \((\S+) .*\)$'.format(patchName,fieldName)

def volAverageRegexp(fieldName):
    return '^\s*volAverage\(\) of {} = (\S+).*$'.format(fieldName)

def parseStdout(astring,regex):
    import re
    pat = re.compile(regex)
    result  = None
    for line in astring.split('\n'):
        match = pat.match(line)
        if match:
            result = match.group(1)
    try:
        return float(result)
    except:
        return 0.0

def parametersFromFileToCsv(fileName,parameters):
    parameterDict = {}
    with open(fileName) as fp:
        for line in fp:
            parameter,value = line.strip().split(':',1)
            if parameter in parameters:
                parameterDict[parameter] = value
    valueString = ','.join(['{}'.format(parameterDict[p]) for p in parameters ])
    return valueString

#   #   #   #   #   #   #   #   #   #  #   #   #   #   #   #   #   #   #   #    

bulkVelocity = 1.0
blDelta = 1.0

csvResultsFile=open('testResults.csv','w')
extractParams = ['__meshtype', '__lesmodel', '__wallfunction', '__k_wallfunction', '__deltamodel', '__basesize', '__divscheme']

for i,case in enumerate(sorted(glob('subCase_*'))):
    caseName = case
    case = os.path.join(os.getcwd(),case)
    print '\nCASE {}'.format(case)

    pavgStdout = runFoamCommand(patchAverageCmd('wallShearStressMean','wallBottom'),case)
    wallShearStress = parseStdout(pavgStdout,patchAverageRegexp('wallShearStressMean','wallBottom'))

    kinematicViscosity = readNu(caseName)
    bl = boundaryLayer(bulkVelocity,blDelta,kinematicViscosity)
    bl.setWallShearStress(wallShearStress)

    vavgStdout = runFoamCommand(volumeAverageCmd('delta'),case)
    vavgDelta = parseStdout(vavgStdout,volAverageRegexp('delta'))

    parametersFile = os.path.join(case,'caseParameters')
    paramVals = parametersFromFileToCsv(parametersFile,extractParams)
    if i == 0: # First round
        csvResultsFile.write('{},'.format('Name'))
        csvResultsFile.write(','.join(extractParams))
        csvResultsFile.write(',')
        csvResultsFile.write(bl.csvHead())
        csvResultsFile.write(',')
        csvResultsFile.write('Avg. LESdelta')
        csvResultsFile.write('\n')
    csvResultsFile.write('{},'.format(caseName))
    csvResultsFile.write(paramVals)
    csvResultsFile.write(',')
    csvResultsFile.write(bl.csv())
    csvResultsFile.write(',')
    csvResultsFile.write(str(vavgDelta))
    csvResultsFile.write('\n')

