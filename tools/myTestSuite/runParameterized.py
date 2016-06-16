#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os,sys,glob,re

from suiteTools import SuiteRunner
from testSuiteUtils import Paths,Error,Debug,Info

progName = os.path.basename(sys.argv[0])

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

import signal
def signal_handler(signal, frame):
    print('\nOapps! Killed by SIGINT.')
    print('\t(Manually clean out stale subCases)')
    sys.exit(0)

def setEnvironment():
    #libDir = os.path.join(Paths().AppRoot,'parameterizeLib')
    libDir = Paths().AppRoot
    sys.path.append(libDir)
    os.environ['PYTHONPATH'] = libDir

def usage():
    print '''
    {0} [path/to/testSuite/or/sub] <norun> <nopresent>
    '''.format(progName)

if __name__=="__main__":
    '''Very raw, at the moment'''

    if not len(sys.argv) > 1:
        usage()
        Error('Path to tests top level as first argument needed')

    tests = sys.argv[1]

    nThreads = 4
    presentationRoot = '/tmp/tests/results/'
    applicationRoot = os.path.realpath(os.path.dirname(sys.argv[0]))
    testRoot = os.getcwd()

    signal.signal(signal.SIGINT, signal_handler)

    # Initiate Paths(Borg) to carry global data
    Paths(testRoot,presentationRoot,applicationRoot)

    setEnvironment()

    Suite = SuiteRunner(tests)

    removeSubCases = True
    if 'nopresent' in sys.argv:
        Paths().deleteSubCases = False
        Paths().skipPresentation = True
    if not 'norun' in sys.argv:
        Suite.run(nThreads)
    Suite.present()
