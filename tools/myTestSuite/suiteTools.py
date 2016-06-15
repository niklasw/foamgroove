#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os,sys,glob,re
from testSuiteUtils import Info,Warning,Error,Debug,Paths
from caseManaging import FoamCase, CaseManager
from caseBook import Book

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class ParamDict(dict):
    ''' Read test config and create parameter matrix from json file.
    config file example:

    {
        "comment1": "Normal comments are not allowed in JSON",
        "comment2": "but comments can be hidden like this...",

        "parameters": {
            "__les_model": [ "SMG", "Smagorinsky" ],
            "__wall_model": [ "nutUSpaldingWallFunction", "zeroGradient" ]
        },
        "groups": {
            "g1": ["__les_model","__wall_model"]
        },
        "solution": {
            "nprocs": 1,
            "solver": "pimpleFoam"
        }
    }
    '''

    mandatorySections_ = ['solution','parameters']

    def __init__(self,fileName):
        import json
        if not os.path.isfile(fileName):
            Error('Fatal: missing test file')
        dict.__init__(self,json.load(open(fileName)))
        self.fileName = fileName
        self._check()

        general = self.get('solution')
        self.solver = general.get('solver')
        self.nCores = general.get('ncores')
        self.parameters = self.get('parameters')
        # List of groups. Discard the keys. Groups are optional.
        if self.has_key('groups'):
            self.groups = self.get('groups').values()
        else:
            self.groups = []

        self.isGrouped = self.checkGroups()

    def _check(self):
        for ms in self.mandatorySections_:
            if not ms in self.keys():
                Error('Fatal: missing section in JSON configuration: {0}'.format(ms))

    def checkGroups(self):
        # Check that groups contain valid parameter names,
        # that they appear only once, and that number of values matches
        params = self.parameters.keys()
        isGrouped = dict(zip(params, len(params)*[False]))
        for g in self.groups:
            if(len(g) < 2):
                Error('Fatal: group has fewer than two entries.')
            for param in g:
                if(not param in params):
                    Error('Fatal: unknown parameter name in group: {0}'.format(param))
                if(isGrouped.get(param)):
                    Error('Fatal: Parameter appears in more than one group: {0}'.format(param))
                isGrouped[param] = True
            l = [len(self.parameters.get(p)) for p in g ]
            if not l or not l.count(l[0]) == len(l):
                Error('Fatal: Grouped parameters have different number of values.')
        return isGrouped

    def parameterMatrix_old(self):
        import itertools
        keyValuePairs = list()
        for param,values in self.parameters.iteritems():
            paramVals = None
            if isinstance(values,list):
                paramVals = zip(len(values)*[param],values)
            else:
                paramVals = [(param,values)]
            keyValuePairs.append(paramVals)
        # Found this on da google...
        # To create an iterable of all combinations
        matrix = itertools.product(*keyValuePairs)
        for row in matrix:
            # Check if row matches grouping criteria
            if self.keepTest(row):
                rowDict = dict(row)
                yield rowDict

    def parameterMatrix(self):
        import itertools
        keyValuePairs = list()

        # Add grouped parameters first as single large tuple
        for g in self.groups:
            groupParamVals = None
            for param in g:
                values = self.parameters.get(param)
                paramVals = None
                if isinstance(values,list):
                    paramVals = zip(len(values)*[param],values)
                else:
                    paramVals = [(param,values)]
                if isinstance(groupParamVals,list):
                    groupParamVals = [a + b for (a,b) in zip(groupParamVals,paramVals)]
                else:
                    groupParamVals = paramVals
            keyValuePairs.append(groupParamVals)

        # Now add non-grouped parameters
        for param,values in self.parameters.iteritems():
            if not self.isGrouped[param]:
                paramVals = None
                if isinstance(values,list):
                    paramVals = zip(len(values)*[param],values)
                else:
                    paramVals = [(param,values)]
                keyValuePairs.append(paramVals)

        # Found this on da google...
        # To create an iterable of all combinations
        matrix = itertools.product(*keyValuePairs)

        for row in matrix:
            # Expand all grouped parameters to pairs
            rowExpand = list()
            for entry in row:
                for i in range(0,len(entry),2):
                    rowExpand.append((entry[i], entry[i+1]))
            yield dict(rowExpand)

class TestRunner:

    def __init__(self,caseRoot,nThreads=1):
        self.case = FoamCase(caseRoot)
        self.case.clean()
        self.nThreads = nThreads
        self.currentConfig = None

    def readConfig(self,testFile):
        '''Need to separate this and carry currentConfig,
           in order to use a generator for test series'''
        self.currentConfig = ParamDict(os.path.join(self.case.root, testFile))

    def generateTests(self):
        '''Yield a manager for every row in the parameter matrix.
        Note: This is a generator. Must be looped for anyting to happen.'''

        for parameterSet in self.currentConfig.parameterMatrix():
            subCase = self.case.mkSubCase()
            worker = CaseManager(subCase, self.currentConfig)
            worker.setParameters(parameterSet)
            yield worker

    def run(self,testFile):
        Info('Running test file {0}\n\tin case {1}'.format(testFile, self.case.root))
        self.readConfig(testFile)
        for worker in self.generateTests():
            worker.do()

    def runParallelTests(self,testFile):
        from multiprocessing import Process
        Info('Running test file {0}\n\tin case {1}'.format(testFile, self.case.root))
        self.readConfig(testFile)
        testGenerator = self.generateTests()
        allDone = False
        while not allDone:
            pss = []
            for i in range(self.nThreads):
                try:
                    worker = testGenerator.next()
                except:
                    allDone = True
                    break
                ps = Process(target=worker.do)
                pss.append(ps)
                ps.start()
                Info('Started process {0}: {1}'.format(i,worker.case.name))

            # Wait for processes to end
            for i,ps in enumerate(pss):
                ps.join()
            Debug('\tAll threads done')

    def runAllTestFiles(self):
        for testFile in self.case.getTestFiles():
            #self.runThreaded(testFile)
            self.runParallelTests(testFile)

    def collectBooks(self):
        '''Iter return a Book, read from each subcase's presentation dir'''
        from testSuiteUtils import findFiles
        pRoot = Paths().presentRoot(self.case.root)
        bookPaths = findFiles(pRoot,Book.dbFile,dirname=True)
        bookPaths.sort()
        for root in bookPaths:
            yield Book.open(root)

    def createTestTable(self):
        from htmlUtils import htmlTable, htmlLink
        Info('Creating test table for {0}'.format(self.case.root))
        tableRows = []
        heads = ['Name','Status','Parameters','Time']
        for caseBook in self.collectBooks():
            summary = caseBook.summaryDict()
            linkPath = os.path.join(caseBook.presentRoot(),'index.html')
            caseLink = htmlLink(summary['name'],linkPath)
            row = [caseLink, summary['status'],summary['parameters'], \
                   summary['time']]
            tableRows.append(row)
        return htmlTable(tableRows,heads)

    def printHtml(self):
        from htmlUtils import htmlDiv,htmlTemplate,htmlSection,htmlList
        div = htmlDiv(cls='test_table')

        header = htmlSection(self.case.name,self.case.root,level=1)
        testTable = self.createTestTable()

        div.append(header.content)
        div.append(testTable.content)
        div.update()

        presentRoot = Paths().presentRoot(self.case.root)
        template = os.path.join(Paths().HtmlTemplates,'testCase.html')
        doc = htmlTemplate(template,root=presentRoot, \
                           relRoot=Paths().PresentRoot)
        doc.addContent(content1=div.content)

        return doc.content

    def writeHtml(self):
        presentRoot = Paths().presentRoot(self.case.root)
        if os.path.isdir(presentRoot):
            Debug('Writing index.html to {0}'.format(presentRoot))
            with open(os.path.join(presentRoot,'index.html'),'w') as fp:
                fp.write(self.printHtml())

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class SuiteRunner:

    def __init__(self, root):
        self.root = os.path.realpath(root)
        if not os.path.isdir(self.root):
            Error('Not a directory: {0}'.format(self.root))
        self.tests = self.findTests()
        if not self.tests:
            Info('No tests found.')

    def findTests(self):
        from testSuiteUtils import findFiles
        return findFiles(self.root,'.*\.json',dirname=True)

    def run(self, nThreads = 1):
        for root in self.tests:
            Info(root)
            Test = TestRunner(root, nThreads)
            Test.runAllTestFiles()

    def present(self):
        from htmlUtils import htmlTree
        if Paths().skipPresentation:
            Info('Presentation skipped')
            return
        for root in self.tests:
            Test = TestRunner(root)
            Test.writeHtml()
        # Create the tree of recursive index.html files
        template = os.path.join(Paths().HtmlTemplates,'testCase.html')
        htTree = htmlTree(Paths().PresentRoot, \
                          Paths().SubCasePattern,template)
        htTree.makeIndexTree()


if __name__=='__main__':
    if not len(sys.argv) > 1:
        Error('Path to tests top level as sole argument needed')

    Suite = SuiteRunner(sys.argv[1])
    for root in Suite.tests:
        Info(root)
        Test = TestRunner(root)

        testFiles = Test.case.getTestFiles()

        for testFile in testFiles:
            config = ParamDict(os.path.join(Test.case.root, testFile))
            matrix = config.parameterMatrix()
            Info('Test matrix from test file {0}:'.format(testFile))
            for parameterSet in matrix:
               print parameterSet
