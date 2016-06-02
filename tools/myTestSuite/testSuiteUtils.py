#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os,sys,glob,re

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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
    return do(sp1,sp2) if len(sp2)>len(sp1) else do(sp2,sp1)

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
    print SuitePaths.presentRoot(os.path.join(os.getcwd(),sys.argv[1]))

    files = findFiles('/tmp/testSuite','caseB.*',dirname=False)
    folders=findFiles('/tmp/testSuite','caseB.*',dirname=True)

    for item in files:
        print item

    print

    for item in folders:
        print item

