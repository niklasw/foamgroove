#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os,sys,glob,re
import ConfigParser
from testSuiteUtils import *
from caseManaging import FoamCase, CaseManager

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class ParamFile(ConfigParser.ConfigParser):
    mandatorySections_ = ['solution','parameters']

    def __init__(self,fileName):
        ConfigParser.ConfigParser.__init__(self)
        self.fileName = fileName
        self.read(fileName)
        self._check()
        self.solver = self._getSolver()
        self.nCores = self._getNCores()
        self.nThreads = self._getNThreads()
        self.parameters = self._getParameters()

    def _check(self):
        for ms in self.mandatorySections_:
            if not ms in self.sections():
                Error('Fatal: missing section in configuration: {0}'.format(ms))

    def _getCleanValues(self,section,key):
        try:
            val = self.get(section,key).split(',')
            val = [ s.strip() for s in val ]
            return filter(bool, val)
        except:
            Error('Failed reading config for {0}:{1}'.format(section,key))

    def _getNCores(self):
        ncores = self._getCleanValues('solution','nProcs')[0]
        return int(ncores)

    def _getNThreads(self):
        nthreads = self._getCleanValues('solution','nThreads')[0]
        return int(nthreads)

    def _getSolver(self):
        return self._getCleanValues('solution','solver')[0]

    def _getParameters(self):
        return self.options('parameters')

    def parameterMatrix(self):
        import itertools
        keyValuePairs = list()
        for param in self._getParameters():
            values = self._getCleanValues('parameters',param)
            paramVals = zip(len(values)*[param],values)
            keyValuePairs.append(paramVals)
        # Found this on da google...
        # To create an iterable of all combinations
        return itertools.product(*keyValuePairs)

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class TestRunner:

    def __init__(self,caseRoot,testFileName):
        self.testFileName = testFileName
        self.config = ParamFile(testFileName)
        self.case = FoamCase(caseRoot)
        self.case.clean()

    def run(self,cleanup=False):
        for parameterSet in self.config.parameterMatrix():
            subCase = self.case.mkSubCase()

            worker = CaseManager(subCase, self.config, deleteAfter=cleanup)
            worker.setParameters(dict(parameterSet))
            worker.do()

    def runThreaded(self,cleanup=False):
        """Not threaded! multi-processed. Threading did not
           work with matplotlib"""
        from multiprocessing import Process

        setIterator = self.config.parameterMatrix()
        while True:
            pss = []
            for i in range(self.config.nThreads):
                try:
                    parameterSet = setIterator.next()
                except:
                    return

                subCase = self.case.mkSubCase()
                worker = CaseManager(subCase, self.config, deleteAfter=cleanup)
                worker.setParameters(dict(parameterSet))
                ps = Process(target=worker.do)
                pss.append(ps)
                ps.start()
                Info('started thread {0}: {1}'.format(i,subCase.name))

            # Wait for threads to end
            for ps in pss:
                ps.join()



# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

if __name__=="__main__":
    progPath = os.path.dirname(sys.argv[0])

    testFileName = 'les.test'

    caseRoot = os.getcwd()
    if len(sys.argv) > 1:
        pathArg = sys.argv[1]
        if os.path.isabs(pathArg):
            caseRoot = pathArg
        else:
            caseRoot = os.path.join(os.getcwd(),os.path.realpath(pathArg))
    print caseRoot

    T = TestRunner(caseRoot, testFileName)
    #T.run(cleanup=False)
    T.runThreaded(cleanup=False)

