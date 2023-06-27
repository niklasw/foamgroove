#!/usr/bin/env python

import os
from interactor2 import interactor

I = interactor()

templatesDir = os.path.join(os.getenv('OpenFOAM_TEMPLATES_DIR'),'new/2.2')

print "Templatres Dir = \n",templatesDir
class simulationType(list):
    base = ('Base',)
    flow = ('Incompressible','Compressible')
    time   = ('Steady','Unsteady')
    geometry = ('Internal','External')
    options = ('None','DynamicMesh')

    typesDefinitions = [['Basic',base],
                        ['Flow Regime', flow],
                        ['Time resolution',time],
                        ['Geometry class',geometry],
                        ['Options', options]]

    def __init__(self):
        list.__init__([])

    def __str__(self):
        for item in self:
            print item

def mkPath(tName):
    t = os.path.join(templatesDir,tName,'')
    if not os.path.isdir(t):
        I.error("Cannot find template directory "+t)
    else:
        return t

def cpTemplate(path,target='.'):
    from subprocess import Popen,PIPE
    from glob import glob
    files = glob(path+'*')
    cmd = ['/bin/cp','-r']+files+[target]
    p = Popen(cmd,stdout=PIPE,stderr=PIPE)
    p.wait()
    out,err=p.communicate();
    I.info(out)
    if err:
        I.error(err)

def askAndFetch(target = '.'):
    S = simulationType()

    for definition in S.typesDefinitions:
        p = definition[0]
        a = definition[1]
        selected = I.iGet(prompt=p,allowed=a,showAllowed=True)
        cpTemplate(mkPath(selected), target)

if __name__ == "__main__":
    askAndFetch()
