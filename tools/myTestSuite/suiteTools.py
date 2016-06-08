#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os,sys,glob,re
from testSuiteUtils import *
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
            "nthreads": 8,
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
        self.nThreads = general.get('nthreads')
        self.parameters = self.get('parameters')
        # List of groups. Discard the keys. Groups are optional.
        if self.has_key('groups'):
            self.groups = self.get('groups').values()
        else:
            self.groups = []

    def _check(self):
        for ms in self.mandatorySections_:
            if not ms in self.keys():
                Error('Fatal: missing section in JSON configuration: {0}'.format(ms))

    def parameterMatrix(self):
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
            yield dict(row)


    def collapseGroup(self,group):
        for test in self.parameterMatrix():
            for parameter in group:
                pass


    def collapseGroups(self):
        # "group1":  ["__velocity","__wall_model"],
        def isUniformList(lst):
            return not lst or lst.count(lst[0]) == len(lst)

        def getTestByKeyVal(key1,key2,value1,value2):
            for row in self.parameterMatrix():
                if row[key1] == value1 and row[key2] == value2:
                    yield row
        for group in self.groups:
                paramVals = self.parameters.get(param)
                pass
        else:
            print 'No Groups'
                
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class TestRunner:

    def __init__(self,caseRoot):
        self.case = FoamCase(caseRoot)
        self.case.clean()

    def run(self,testFile, cleanup=False):
        '''Run all rows in the parameter matrix'''

        Info('Running test file {0} in case {1}'.format(testFile, self.case.root))

        config = ParamDict(os.path.join(self.case.root, testFile))

        for parameterSet in config.parameterMatrix():
            subCase = self.case.mkSubCase()

            worker = CaseManager(subCase, config, deleteAfter=cleanup)
            #worker.setParameters(dict(parameterSet))
            worker.setParameters(parameterSet)
            worker.do()

    def runThreaded(self,testFile,cleanup=False):
        '''Not threaded! multi-processed. Threading did not
           work with matplotlib in the post scripts'''
        from multiprocessing import Process

        Info('Running test file {0} in case {1}'.format(testFile, self.case.root))

        config = ParamDict(os.path.join(self.case.root,testFile))

        setIterator = config.parameterMatrix()
        allParamsDone = False
        caseRoots = []
        while not allParamsDone:
            pss = []
            for i in range(config.nThreads):
                try:
                    parameterSet = setIterator.next()
                except:
                    allParamsDone = True
                    break

                subCase = self.case.mkSubCase()
                worker = CaseManager(subCase, config, deleteAfter=cleanup)
                worker.setParameters(dict(parameterSet))
                ps = Process(target=worker.do)
                pss.append(ps)
                ps.start()
                Info('started thread {0}: {1}'.format(i,subCase.name))

            # Wait for processes to end
            for i,ps in enumerate(pss):
                ps.join()
            Debug('\tAll threads done')

    def runAllTestFiles(self,cleanup=False):
        for testFile in self.case.getTestFiles():
            self.runThreaded(testFile, cleanup=cleanup)

    def collectBooks(self):
        '''Iter return a Book, read from each subcase's presentation dir'''
        pRoot = BorgPaths().presentRoot(self.case.root)
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
        
        presentRoot = BorgPaths().presentRoot(self.case.root)
        template = os.path.join(BorgPaths().HtmlTemplates,'testCase.html')
        doc = htmlTemplate(template,root=presentRoot, \
                           relRoot=BorgPaths().PresentRoot)
        doc.addContent(content1=div.content)

        return doc.content

    def writeHtml(self):
        presentRoot = BorgPaths().presentRoot(self.case.root)
        if os.path.isdir(presentRoot):
            Debug('Writing index.html to {0}'.format(presentRoot))
            with open(os.path.join(presentRoot,'index.html'),'w') as fp:
                fp.write(self.printHtml())

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class SuiteRunner:

    def __init__(self, argv):
        self.root = os.path.realpath(argv[1])
        if not os.path.isdir(self.root):
            Error('Not a directory: {0}'.format(self.root))
        self.tests = self.findTests()

    def findTests(self):
        return findFiles(self.root,'.*\.json',dirname=True)

    def run(self):
        for root in self.tests:
            Info(root)
            Test = TestRunner(root)
            Test.runAllTestFiles(cleanup=1)

    def present(self):
        from htmlUtils import htmlTree
        for root in self.tests:
            Test = TestRunner(root)
            Test.writeHtml()
        # Create the tree of recursive index.html files
        template = os.path.join(BorgPaths().HtmlTemplates,'testCase.html')
        htTree = htmlTree(BorgPaths().PresentRoot, \
                          BorgPaths().SubCasePattern,template)
        htTree.makeIndexTree()


if __name__=='__main__':
    if not len(sys.argv) > 1:
        Error('Path to tests top level as sole argument needed')

    Suite = SuiteRunner(sys.argv)
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
