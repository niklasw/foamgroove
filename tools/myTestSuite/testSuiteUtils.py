#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os,sys,glob,re

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class Borg:
    '''Hivemind base class. Google "python borg singleton" '''
    _shared_state = {}
    def __init__(self):
        self.__dict__ = self._shared_state

class Paths(Borg):
    '''By subclassing Borg, this class variables are common to
    all instances of Path(), in the main process'''

    SubCasePrefix = 'subCase_'
    SubCasePattern= SubCasePrefix+'.*'

    def __init__(self, testRoot=None, presentRoot=None, appRoot=None ):
        Borg.__init__(self)
        if testRoot:
            self.TestRoot = testRoot        # Top level test root
        if appRoot:
            self.AppRoot = appRoot          # CWD at app launch
        if presentRoot:
            self.PresentRoot = presentRoot  # Top level presentation root
        self.assertRootsDefined()
        self.HtmlTemplates = os.path.join(self.AppRoot,'htmlTemplates')
        self.skipPresentation = False
        self.deleteSubCases = True

    def assertRootsDefined(self):
        for root in ['TestRoot','PresentRoot','AppRoot']:
            try:
                getattr(self,root)
            except(AttributeError):
                Error('Missing path in Paths: {}'.format(root))
        if not os.path.isdir(self.TestRoot):
            Error('TestRoot does not exist: {}'.format(self.TestRoot))
        if not os.path.isdir(self.PresentRoot):
            # A safety measure. User makes sure the presentation root exist.
            Error('PresentRoot does not exist: {}'.format(self.PresentRoot))

    def presentRoot(self,testRoot):
        fullPath = os.path.realpath(testRoot)
        tpL = len(self.TestRoot)+1
        relativeTestRoot = fullPath[tpL:]
        return os.path.join(self.PresentRoot,relativeTestRoot)

    def mkPresentRoot(self,testRoot):
        '''Create and return path to tests presentation directory'''
        pRoot = self.presentRoot(testRoot)
        try:
            Debug('Present root ='+pRoot)
            if os.path.exists(pRoot): # FIXME not safe could remove home...:-|
                Debug('Removing all files in {0}'.format(pRoot))
                (os.remove(f) for f in os.listdir(pRoot))
            else:
                os.makedirs(pRoot)
        except:
            Error('Cannot create presentation root dir {0}'.format(pRoot))
        return pRoot


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

def Info(astring):
    print '>>',astring

def Debug(astring):
    #Info('DEBUG: '+astring)
    return

def Error(s=''):
    import inspect
    errFile = inspect.stack()[1][1]
    errLine =  inspect.stack()[1][2]
    errCaller = inspect.stack()[1][3]
    print 'Error() called from Function {0}\nin {2} line {1}'.format(errCaller,errLine,errFile)
    print 'ERROR:',s
    sys.exit(1)

def Warning(s=''):
    print 'WARNING:',s

def leastCommonPath(p1,p2):
    import os
    def do(a1,a2):
        lcp = []
        for i,item in enumerate(a1):
            if item == a2[i]:
                lcp.append(item)
            else:
                break
        return os.path.join(*lcp)

    sp1 = os.path.realpath(p1).split(os.sep)
    sp2 = os.path.realpath(p2).split(os.sep)
    common = do(sp1,sp2) if len(sp2)>len(sp1) else do(sp2,sp1)
    return os.path.sep+common

def cleanPostProcessingData(postProcessingFile):
    """Cleans e.g. forces files from parentheses and comments.
    Returns a list of strings void of parentheses (etc)"""
    import string
    stringList = None
    if not os.path.isfile(postProcessingFile):
        Error('Cannot read data file {}'.format(postProcessingFile))
    with open(postProcessingFile,'r') as fp:
        stringList = fp.readlines()
    dropPat = re.compile('^\s*(?![\(\)\#"\/a-zA-Z])')
    parenPat = re.compile('[,\(\)]')
    filtered = filter(dropPat.match, stringList)
    filtered = [parenPat.sub(' ',a) for a in filtered]
    return map(string.strip,filtered)

def findFiles(startRoot, patternString, dirname=False, unique=False):
    '''Return dirs or file paths containing file matching patternString.
    If dirname=True containing dir is returned, otherwise the matching
    file path. unique only speeds up the search...?'''
    import re
    pat = re.compile(patternString)
    paths = []
    for root, dirs, files in os.walk(startRoot):
        for f in files:
            if pat.match(f):
                if dirname:
                    paths.append(root)
                    break
                else:
                    paths.append(os.path.join(root,f))
                    if unique: break
    return list(set(paths)) # Just in case there are still duplicates...

def findDirs(startRoot, patternString, dirname=False, unique=False):
    '''Return dirs containing dir matching patternString. If dirname=
    True containing dir is returned, otherwise the matching dir path.
    unique only speeds up the search...?'''
    import re
    pat = re.compile(patternString)
    paths = []
    for root, dirs, files in os.walk(startRoot):
        for d in dirs:
            if pat.match(d):
                if dirname:
                    paths.append(root)
                    break
                else:
                    paths.append(os.path.join(root,d))
                    if unique: break
    return list(set(paths)) # Just in case there are still duplicates...


if __name__=='__main__':

    files = findFiles('/tmp/testSuite','caseB.*',dirname=False)
    folders=findFiles('/tmp/testSuite','caseB.*',dirname=True)

    for item in files:
        print item

    print

    for item in folders:
        print item

    print 'Present root'

    tr = os.getcwd()

    p1 = Paths(appRoot=sys.argv[0], testRoot = tr, presentRoot = sys.argv[1])

    p2 = Paths()

    print p1.presentRoot(tr+'/rasTests/case0')

    p2.PresentRoot = '/tmp/tests2'

    print p1.presentRoot(tr+'/lesTests/case0')
    print p2.presentRoot(tr+'/lesTests/case1')
