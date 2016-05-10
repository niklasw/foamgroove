#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os,sys,glob,re

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

def Info(astring):
    print '>>',astring

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



