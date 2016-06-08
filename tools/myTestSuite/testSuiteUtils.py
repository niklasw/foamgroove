#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os,sys,glob,re

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class Borg:
    '''Hivemind base class'''
    _shared_state = {}
    def __init__(self):
        self.__dict__ = self._shared_state

class BorgPaths(Borg):
    
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
        HtmlTemplates = os.path.join(self.AppRoot,'htmlTemplates')

    def assertRootsDefined(self):
        for root in [self.TestRoot,self.PresentRoot,self.AppRoot]:
            if not root:
                Error('Missing path in BorgPaths: {}'.format(root))
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



class SuitePaths:
    PresentRoot = '/tmp/testSuite/results'
    AppRoot = os.path.realpath(os.path.dirname(sys.argv[0]))
    HtmlTemplates = os.path.join(AppRoot,'htmlTemplates')
    SubCasePrefix = 'subCase_'
    SubCasePattern= SubCasePrefix+'.*'

    @staticmethod
    def presentRoot(currentRoot):
        """Create presentation root from self.caseRoot"""
        realCurrentRoot = os.path.realpath(currentRoot)
        aPL = len(SuitePaths.AppRoot)
        appRelativeRoot = realCurrentRoot[aPL+1:]
        pRoot = os.path.join(SuitePaths.PresentRoot,appRelativeRoot)
        return  pRoot

    @staticmethod
    def presentRoot2(currentRoot):
        """Create presentation root from self.caseRoot"""
        realCurrentRoot = os.path.realpath(currentRoot)
        aPL = len(SuitePaths.AppRoot)
        appRelativeRoot = realCurrentRoot[aPL+1:]
        pRoot = os.path.join(SuitePaths.PresentRoot,appRelativeRoot)
        return  pRoot

    @staticmethod
    def mkPresentRoot(currentRoot):
        pRoot = SuitePaths.presentRoot(currentRoot)
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
    with open(postProcessingFile,'r') as fp:
        stringList = fp.readlines()
    dropPat = re.compile('^\s*(?![\(\)\#"\/a-zA-Z])')
    parenPat = re.compile('[,\(\)]')
    filtered = filter(dropPat.match, stringList)
    filtered = [parenPat.sub(' ',a) for a in filtered]
    return map(string.strip,filtered)

def findFiles(startRoot, patternString, dirname=False, unique=False):
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
    import re
    pat = re.compile(patternString)
    paths = []
    for root, dirs, files in os.walk(startRoot):
        for f in dirs:
            if pat.match(f):
                if dirname:
                    paths.append(root)
                    break
                else:
                    paths.append(os.path.join(root,f))
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

    tr = '/home/soft/OpenFOAM/foamGroove/tools/myTestSuite'

    p1 = BorgPaths(appRoot=sys.argv[0], testRoot = tr, presentRoot = sys.argv[1])

    p2 = BorgPaths()

    print p1.presentRoot(tr+'/rasTests/case0')

    p2.PresentRoot = '/tmp/tests2'

    print p1.presentRoot(tr+'/lesTests/case0')
    print p2.presentRoot(tr+'/lesTests/case1')
