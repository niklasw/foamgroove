#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os,sys,glob,re
from testSuiteUtils import *
from caseBook import Book

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class FoamCase:

    def __init__(self,caseRoot,skipNames=['.*\.test','.*.html','.*\.?log','.*\.json']):
        self.root = caseRoot
        skipNames.append(Paths.SubCasePattern)
        self.skipNames = skipNames
        self.skipNames.append('.*{0}.*'.format(Paths.SubCasePrefix))
        self.skipPatterns = [ re.compile(p) for p in skipNames ]
        self.fileList = list(self._getFileTreeList())
        self.subRoots = []
        self.name = os.path.split(caseRoot)[-1]
        self.parameters = {}

    def getTestFiles(self):
        return findFiles(self.root,'.*\.json')

    def _getFileTreeList(self):
        def filterNames(fileNames):
            for p in self.skipPatterns:
                fileNames = [ f for f in fileNames if not p.match(f) ]
            return fileNames

        for root,dirs,files in os.walk(self.root):
            for f in filterNames(files):
                fPath = os.path.join(root,f)
                yield fPath

    def clean(self):
        import shutil
        subRoots = glob.glob(Paths.SubCasePrefix+'*')
        for subCase in subRoots:
            shutil.rmtree(subCase)

    def mkSubCase(self):
        '''Create a copy of this case,avoiding to duplicate files
        matching patterns in self.skipNames. Also avoiding already
        present subRoots.'''
        import shutil
        subCaseName = '{0}{1:03d}'.format(Paths.SubCasePrefix,len(self.subRoots))
        subCaseRoot = os.path.join(self.root,subCaseName)

        if os.path.isdir(subCaseRoot):
            shutil.rmtree(subCaseRoot)

        # Fun way to return false if path matches none of the skipPatterns
        keepMe = lambda f: all([not p.match(f) for p in self.skipPatterns])
        caseContent = [a for a in os.listdir(self.root) if keepMe(a)]

        os.mkdir(subCaseRoot)

        for item in caseContent:
            source = os.path.join(self.root,item)
            target = os.path.join(subCaseRoot,item)
            if os.path.isdir(source):
                shutil.copytree(source,target)
            else:
                shutil.copy(source,target)
        self.subRoots.append(subCaseRoot)
        return FoamCase(subCaseRoot)

    def annotate(self,astring):
        with open(os.path.join(self.root,'caseParameters'),'a') as fp:
            fp.write('{0}\n'.format(astring))

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class CaseManager:

    def __init__(self,foamCase,config):
        self.config = config
        self.case = foamCase
        self.delete = Paths().deleteSubCases
        self.book = Book(foamCase.root)

    def setParameters(self,parametersDict):
        from string import Template
        self.parameters = parametersDict
        self.book.parameters = parametersDict

        for value in parametersDict.values():
            self.case.name += '__'+str(value)
        result = ''
        for f in self.case.fileList:
            with open(f,'r') as fp:
                tpl = Template(fp.read())
                result = tpl.safe_substitute(**parametersDict)
            with open(f,'w') as fp:
                fp.write(result)
        for parameter,value in parametersDict.iteritems():
            self.case.annotate('{0}:{1}'.format(parameter,value))

    def runCommand(self,name='Noname',cmd='',writeToBook = True):
        """Call command through Popen and store data
        in 'Book' """
        import subprocess,time
        startT = time.time()
        p = subprocess.Popen(cmd,                    \
                             stdout=subprocess.PIPE, \
                             stderr=subprocess.PIPE, \
                             cwd=self.case.root,     \
                             shell=True)
        out,err = p.communicate()
        status =  p.wait()
        elapsedT = time.time()-startT

        if writeToBook:
            # Write output messages in the Book
            self.book.timer[name] = elapsedT
            self.book.logData[name] = out
            self.book.errData[name] = err
            self.book.exitStatus[name] = status
        return (out,err,status,elapsedT)

    def preProcess(self):
        script = os.path.join(self.case.root,'prepare.sh')
        if os.path.isfile(script):
            self.runCommand(cmd=script,name='preProcess')
        if self.config.nCores > 1:
            self.runCommand(cmd=['decomposePar','-force'],name='decompose')

    def run(self):
        cmd = []
        if self.config.nCores > 1:
            cmd.extend(['mpirun','-np '+str(self.config.nCores)])
            cmd.extend([self.config.solver,'-parallel'])
        else:
            cmd.append(self.config.solver)
        runCmd = ' '.join(cmd)
        self.runCommand(cmd=runCmd,name='run')

    def finalise(self):
        script = os.path.join(self.case.root,'finalise.sh')
        if os.path.isfile(script):
            self.runCommand(cmd=script,name='finalise')
        if self.config.nCores > 1:
            self.runCommand(cmd=['reconstructPar'],name='reconstruct')
        # Need to write book to disk now, so post processing can add to it.
        # The post processing app, in turn, should open the book from file
        # in order to add pictures and data tables etc.
        self.book.close()

    def postProcess(self):
        script = os.path.join(self.case.root,'post.py')
        out,err,status,etime = self.runCommand(cmd=script,writeToBook=False)

        # Need to re-open Book to write post process run data to it, since
        # it was closed for external app writing by finalise()
        self.book = Book.open(self.case.root)
        self.book.logData['postProcess'] = out
        self.book.errData['postProcess'] = err
        self.book.exitStatus['postProcess'] = status
        self.book.timer['postProcess'] = etime

    def do(self):
        import shutil
        self.preProcess()
        self.run()
        self.finalise()
        self.postProcess()
        self.book.present()

        if self.delete:
            Debug('DELETE {0}'.format(self.case.root))
            shutil.rmtree(self.case.root)
        return True

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
