#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os,sys,glob,re
import ConfigParser
from testSuiteUtils import *


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class FoamCase:
    subCasePrefix = 'subCase_'

    def __init__(self,caseRoot, skipNames=['.*\.test','.*\.?log']):
        self.root = caseRoot
        skipNames.append(self.subCasePrefix+'.*')
        self.skipNames = skipNames
        self.skipPatterns = [ re.compile(p) for p in skipNames ]
        self.fileList = list(self._getFileTreeList())
        self.subcases = []
        self.name = os.path.split(caseRoot)[-1]
        self.parameters = {}

    def _getFileTreeList(self):
        def filterNames(fileNames):
            for p in self.skipPatterns:
                fileNames = [ f for f in fileNames if not p.match(f) ]
            return fileNames

        for root, dirs, files in os.walk(self.root):
            for f in filterNames(files):
                fPath = os.path.join(root,f)
                yield fPath

    def clean(self):
        import shutil
        subCases = glob.glob(self.subCasePrefix+'*')
        for subCase in subCases:
            shutil.rmtree(subCase)


    def mkSubCase(self):
        import shutil
        subCaseName = '{0}{1:03d}'.format(self.subCasePrefix,len(self.subcases))
        subCaseRoot = os.path.join(self.root,subCaseName)
        if os.path.isdir(subCaseRoot):
            shutil.rmtree(subCaseRoot)

        caseContent = glob.glob('*')
        for item in caseContent:
            for p in self.skipPatterns:
                if p.match(item):
                    caseContent.remove(item)

        os.mkdir(subCaseRoot)

        for item in caseContent:
            target = os.path.join(subCaseRoot,item)
            if os.path.isdir(item):
                shutil.copytree(item,target)
            else:
                shutil.copy(item,target)
        self.subcases.append(subCaseRoot)
        return FoamCase(subCaseRoot)

    def setParameters(self,parametersDict):
        self.parameters = parametersDict
        from string import Template
        for value in parametersDict.values():
            self.name += '__'+value
        result = ''
        for f in self.fileList:
            with open(f,'r') as fp:
                tpl = Template(fp.read())
                result = tpl.safe_substitute(**parametersDict)
            with open(f,'w') as fp:
                fp.write(result)
        for parameter, value in parametersDict.iteritems():
            self.annotate('{0}:{1}'.format(parameter,value))

    def annotate(self, astring):
        with open(os.path.join(self.root,'caseParameters'),'a') as fp:
            fp.write('{0}\n'.format(astring))


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class ResultPicture:
    """Picture thing that also can store pictures description
    and it's underlaying data set"""

    from numpy import array
    def __init__(self, root='',fileName='',description='',data=array([])):
        self.root = root if root else os.getcwd()
        self.fileName = fileName
        self.description = description
        self.dataSet = data

    def path(self):
        return os.path.join(self.root,self.fileName)

    def htmlPrint(self):
        """ Could build it's own html div... """
        s = '<div id="test_picture">'
        s+= '<img src="{0}" alt="{1}" />'.format(self.path(),self.fileName)
        s+= '<p>{0}</p>'.format(self.description)
        s+= '</div>'
        return s

class DataTable:
    """Dict like. Store and present tabular data"""

    def __init__(self,data=array(),colNames=[]):
        self.dataDict = data
        self.columnNames = colNames
        self.assert()

    def assert(self):
        if len(self.dataDict.keys()) == 0:
            return True
        else:
            firstKey = self.dataDict.keys()[0]
            rowLen = len(self.dataDict[firstKey])
            for key in self.dataDict.keys():
                if curRowLen == len(self.dataDict[key]):
                    continue
                else:
                    return False
    def fixColNames(self):
        pass




class Book:
    """Object keeping data saved for presenting
       for each subCase. Reponsible for carrying result
       data and case description to and from a file"""

    dbFile = 'caseBook'

    @staticmethod
    def open(root):
        """Helper function for external py's like post processing app
        to access the same book through a pickle db file.
        If the db file does not exist, return an empty Book. Hmm..."""
        import pickle
        dbPath = os.path.join(root,Book.dbFile)
        if os.path.isfile(dbPath):
            print 'Current db path at open =',dbPath
            book = pickle.load(open(dbPath,'rb'))
            return book
        else:
            return Book(root)

    def __init__(self,caseRoot):
        self.description = 'Add a file named description.txt to the case.'
        self.logData = dict()    # String dict with keys by command type name
        self.errData = dict()    # String dict with keys by command type name
        self.exitStatus = dict() # Integer dict with keys by command type name
        self.pictures = list()   # List of ResultPicture
        self.dataTable = 
        self.root = caseRoot
        readCaseDescription()

    def readCaseDescription(self):
        descFile = os.path.join(self.root,'description.txt')
        if os.path.isfile(descFile):
            with open(descFile,'r') as fp:
                self.description = fp.read()

    def presentRoot(self):
        """Create presentation root from self.caseRoot"""
        return self.root #FIXME
        pass

    def changeRoot(self, newRoot):
        """May be of use when switching to presentation dir"""
        self.root = newRoot
        for item in pictures:
            item.root = newRoot

    def close(self):
        """Put the Book back in the db"""
        import pickle
        dbPath = os.path.join(self.root,Book.dbFile)
        print 'Current shelve path at close =',dbPath
        pickle.dump(self,open(dbPath,'wb'))

    def dump(self):
        """Print out all data"""
        pass

    def htmlPrint(self):
        """ Could build it's own html div """
        pass


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


class CaseManager:

    def __init__(self,foamCase, config, deleteAfter=False):
        self.config = config
        self.case = foamCase
        self.delete = deleteAfter
        self.caseBook = Book(foamCase.root)

    def setParameters(self,parametersDict):
        self.parameters = parametersDict

        from string import Template
        for value in parametersDict.values():
            self.case.name += '__'+value
        result = ''
        for f in self.case.fileList:
            with open(f,'r') as fp:
                tpl = Template(fp.read())
                result = tpl.safe_substitute(**parametersDict)
            with open(f,'w') as fp:
                fp.write(result)
        for parameter, value in parametersDict.iteritems():
            self.case.annotate('{0}:{1}'.format(parameter,value))


    def runCommand(self,name='Noname', cmd=''):
        """Call command through Popen and store data
        in 'Book' """
        import subprocess
        p = subprocess.Popen(cmd,                    \
                             stdout=subprocess.PIPE, \
                             stderr=subprocess.PIPE, \
                             cwd=self.case.root,     \
                             shell=True)
        out,err = p.communicate()
        status =  p.wait()

        # Write output messages in the Book
        self.caseBook.logData[name] = out
        self.caseBook.errData[name] = err
        self.caseBook.exitStatus[name] = status
        return (out,err,status)

    def preProcess(self):
        script = os.path.join(self.case.root,'prepare.sh')
        if os.path.isfile(script):
            self.runCommand(cmd=script,name='preProcess')
        if self.config.nCores > 1:
            self.runCommand(cmd=['decomposePar','-force'],name='decompose')

    def run(self):
        cmd = []
        if self.config.nCores > 1:
            cmd.extend(['mpirun', '-np '+str(self.config.nCores)])
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
            self.runCommand(cmd=['reconstructPar'], name='reconstruct')

    def postProcess(self):
        script = os.path.join(self.case.root,'post.py')
        out,err,status = self.runCommand(cmd=script,name='postProcess')

        # Need to re-open Book to write post process run data to it...
        book = Book.open(self.case.root)
        book.logData['postProcess'] = out
        book.errData['postProcess'] = err
        book.exitStatus['postProcess'] = status
        book.close()


    def do(self):
        import shutil
        self.preProcess()
        self.run()
        self.finalise()
        # Need to close book now, so post processing app can add to it...
        self.caseBook.close()
        self.postProcess()
        if self.delete:
            print 'DELETE',self.case.root
            shutil.rmtree(self.case.root)
        return True

