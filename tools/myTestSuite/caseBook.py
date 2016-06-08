#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os,sys,glob,re
from testSuiteUtils import *

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class ResultPicture:
    """Picture thing that also can store pictures description
    and it's underlaying data set"""

    from numpy import array
    def __init__(self, root='',fileName='',description='',data=array([])):
        self.root = root if root else os.getcwd()
        self.fileName = fileName
        self.description = description
        # Optionally store the data behind the picture, for re-use
        # It will be written to filename.dat upon copyTo.
        self.dataSet = data

    def path(self):
        return os.path.join(self.root,self.fileName)

    def copyTo(self,target):
        """Copy picture file and write data set to target"""
        import shutil
        from numpy import savetxt
        if not os.path.exists(target): # FIXME not safe
            os.makedirs(target)
        if not target == self.root:
            shutil.copy(self.path(), target)

        self.root = target

        # Write stored data to file at the target location
        if self.dataSet.size:
            datFile =os.path.join(target,self.fileName+'.dat') 
            savetxt(datFile,self.dataSet.transpose())

    def printHtml(self):
        from htmlUtils import htmlImage
        image = htmlImage(self.fileName,self.path(),'book_picture',self.description)
        return image.content

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class DataTable:
    """Dict like. Store and present tabular data. The data set is a
    list of list: [row0, row1,...], that is, row major..."""

    def __init__(self,data=[[]], columnNames=[]):
        self.data = data
        self.columnNames = columnNames
        self.description = ''
        self.fill()

    def _assert(self):
        rowLengths = [len(row) for row in self.data]
        maxL,minL = (max(rowLengths), min(rowLengths))
        return True if maxL == minL else False

    def fill(self):
        """Fill rows that are shorter than max(len(row)) with mock data"""
        from string import ascii_uppercase,ascii_lowercase
        if not self._assert():
            rowLengths = [len(row) for row in self.data]
            maxL = max(rowLengths)
            headerDone = False
            letters = iter(list(ascii_uppercase+ascii_lowercase))
            for row in self.data:
                while len(row) < maxL:
                    row.append('--')
                if not headerDone:
                    headerDone = True
                    while len(row) < maxL:
                        self.columnNames.append(letters.next())

    def printHtml(self):
        from htmlUtils import htmlTable
        table = htmlTable(self.data,self.columnNames,'book_table',self.description)
        #table.generate()
        return table.content


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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
            Debug('Current db path at open = '+dbPath)
            book = pickle.load(open(dbPath,'rb'))
            return book
        else:
            Debug('Opening empty Book at db path at open = '+dbPath)
            return Book(root)

    def __init__(self,caseRoot):
        self.description = 'Add a file named description.txt to the case.'
        self.logData = dict()    # String dict with keys by command type name
        self.errData = dict()    # String dict with keys by command type name
        self.timer = dict()    # float dict with keys by command type name
                                 # storing time spent in subprocesses for each
                                 # command type ("run", "preProcess"...)
        self.exitStatus = dict() # Integer dict with keys by command type name
        self.pictures = list()   # List of ResultPicture
        self.parameters = {}
        self.dataTables = [] 
        self.root = caseRoot
        self.name = os.path.basename(caseRoot)
        self._readCaseDescription()

    def _readCaseDescription(self):
        descFile = os.path.join(self.root,'description.txt')
        if os.path.isfile(descFile):
            with open(descFile,'r') as fp:
                self.description = fp.read()

    def _makePresentRoot(self):
        return SuitePaths.mkPresentRoot(self.root)

    def presentRoot(self):
        return SuitePaths.presentRoot(self.root)

    def _changeRoot(self, newRoot):
        """May be of use when switching to presentation dir"""
        self.root = newRoot
        if not os.path.exists(newRoot):
            os.makedirs(newRoot)

    def close(self, root=''):
        """Put the Book back in the db"""
        import pickle
        if not root:
            root = self.root
        dbPath = os.path.join(root,Book.dbFile)
        pickle.dump(self,open(dbPath,'wb'))

    def dump(self):
        """Print out all data"""
        pass

    def parameterString(self):
        return '; '.join(map(str,self.parameters.values()))

    def summaryDict(self):
        OK = 'OK'
        if sum(self.exitStatus.values()):
            OK = 'Failed'

        summary={'name':self.name}
        summary['status'] = OK
        summary['parameters'] = self.parameterString()
        #for param,value in self.parameters.iteritems():
        #    summary[param] = value
        summary['time'] = self.timer['run']
        return summary

    def printHtml(self):
        """Creates string with complete html document from template"""
        from htmlUtils import htmlDiv,htmlTemplate,htmlSection,htmlList

        div = htmlDiv(cls='result_book')

        header = htmlSection(self.name,self.description,level=1)

        params =htmlSection('Paramters',self.parameterString(),level=2)

        procTimes = []
        for proc in self.timer.keys():
            procTimes.append('{0}: {1:0.2e} s'.format(proc,self.timer[proc]))
        timesList = htmlList(procTimes,name='Case processing times')

        runTime = htmlSection('Time to run',timesList.content,level=3)
        
        div.append(header.content)
        div.append(params.content)
        div.append(runTime.content)
        
        for pic in self.pictures:
            div.append(pic.printHtml())

        for table in self.dataTables:
            div.append(table.printHtml())

        div.update()
        
        template = os.path.join(SuitePaths.HtmlTemplates,'testCase.html')
        doc = htmlTemplate(template)
        doc.addContent(content1=div.content)

        return doc.content

    def printLogs(self):
        from htmlUtils import htmlDiv, htmlTemplate, htmlPre
        div1 = htmlDiv(cls='log_book')
        for key in self.logData:
            p = htmlPre(key,self.logData[key],level=2)
            div1.append(p.content)
        div1.update()

        div2 = htmlDiv(cls='log_book_err')
        for key in self.errData:
            p = htmlPre(key,self.errData[key],level=2)
            div2.append(p.content)
        div2.update()

        template = os.path.join(SuitePaths.HtmlTemplates,'testCase.html')
        doc = htmlTemplate(template)
        doc.addContent(content1=div1, content2=div2)

        return doc.content

    def writeHtml(self):
        Debug('Writing index.html to {0}'.format(self.presentRoot()))
        with open(os.path.join(self.presentRoot(),'index.html'),'w') as fp:
            fp.write(self.printHtml())

    def writeLogs(self):
        Debug('Writing logs.html to {0}'.format(self.presentRoot()))
        with open(os.path.join(self.presentRoot(),'logs.html'),'w') as fp:
            fp.write(self.printLogs())

    def present(self):
        presentationRoot = self._makePresentRoot()
        for pic in self.pictures:
            pic.copyTo(presentationRoot)
        self.writeHtml()
        self.writeLogs()
        self.close(root=presentationRoot)


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

