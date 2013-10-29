#!/usr/bin/env python

import sys,os
from optparse import OptionParser
from interactor2 import interactor

from subprocess import Popen,PIPE

I = interactor()

class caseTemplate:
    def __init__(self, config, name='',archive=''):
        self.name=name
        self.tgz=archive
        self.config = config
        if not self.find():
            I.error('Template file %s does not exist' % self.tgz)

        self.simulationType = simulationType()
        if not self.simulationType.fromStr(name):
            I.error('Exiting.')

    def find(self):
        if not os.path.isfile(self.tgz):
            return False
        return True

    def tarextract(self,archive='', subFolders=1):
        import tarfile
        pass

    def extract(self):
        tarcmd = ['tar']
        inCase = False
        if os.path.isdir('constant/polyMesh'):
            inCase = True
            tarcmd += ['--strip-components','1']
        tarcmd += ['-xf',self.tgz]

        I = interactor()

        yes = False
        if inCase:
            yes = I.yesno('OK to extract template files onto existing OpenFOAM case?', default='n')
        else:
            I.warn('Cannot find constant/polyMesh. Assuming full case extraction')
            yes = I.yesno('OK to extract a complete case. (Empty mesh)?', default = 'y')

        if yes:
            if not inCase and os.path.isdir(self.name):
                I.error('Case (or at least directory) %s already exists here.' % (self.name))
            else:
                p = Popen(tarcmd,stdout=PIPE,stderr=PIPE)
                retval = p.wait()
                if not retval == 0:
                    I.error('Extraction failed for some reason. Tar returned %i' % retval)
        else:
            I.warn('Nothing done')
            return

class simulationType:
    def __init__(self, time='steady',flow='incompressible',geom='internal'):
        self.geom = self.up(geom)
        self.flow = self.up(flow)
        self.time = self.up(time)

    def up(self,s):
        return s[0].upper()+s[1:].lower()

    def __str__(self):
        return self.geom+self.time+self.flow

    def fromStr(self,S):
        import re
        L = re.sub('([A-Z])',r' \1',S).split()
        if len(L) == 3:
            self.geom,self.flow,self.time=L
            return True
        else:
            I.info('Not reconised as a template: %s' %S)
            return False

class config:

    def __init__(self,name=''):
        self.template = None
        self.templatesDir = os.getenv('OpenFOAM_TEMPLATES_DIR')
        self.checkEnv()
        self.templates = self.listTemplates()


    def checkEnv(self):
        ofVersion = os.getenv('WM_PROJECT_VERSION')
        if not ofVersion:
            I.error('OpenFOAM might not be initiated')
        elif ofVersion[0] < '2':
            I.error('OpenFOAM version not > 2.')
        if not os.path.isdir(self.templatesDir):
            I.error('Templates dir %s does not exist.' % self.templatesDir)
        else:
            return self.templatesDir

    def listTemplates(self):
        dirs = os.listdir(self.templatesDir)
        return dirs

    def getArgs(self):

        def argError(s):
            I.info('Argument Error: '+ s)
            I.info('Run with option --help')
            sys.exit(1)

        self.parser.usage = "%prog"
        opt,args = self.parser.parse_args()

        self.setTemplate(args[0])

def apply():
    from interactor2 import interactor
    conf = config()

    selectedFile, templateName = I.iFileSelector( prompt='Select template:',
                                                  path=conf.templatesDir,
                                                  suffix='.tgz',
                                                  stripShow=True)

    caseTemplate(conf,templateName,selectedFile).extract()

if __name__=='__main__':
    apply()


