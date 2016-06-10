#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os,sys,glob,re
from suiteTools import SuiteRunner
from testSuiteUtils import *

progName = os.path.basename(sys.argv[0])

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

import signal
def signal_handler(signal, frame):
    print('\nOapps! Killed by SIGINT.')
    print('\t(Manually clean out stale subCases)')
    sys.exit(0)

def usage():
    print '''
    {0} [path/to/testSuite/or/sub] <norun> <nopresent>
    '''.format(progName)

if __name__=="__main__":

    if not len(sys.argv) > 1:
        usage()
        Error('Path to tests top level as first argument needed')

    signal.signal(signal.SIGINT, signal_handler)

    # Initiate Paths(Borg) to carry global data
    Paths(testRoot    = os.getcwd(), \
          presentRoot = '/tmp/testSuite/results/', \
          appRoot     = os.path.dirname(sys.argv[0]))

    Suite = SuiteRunner(sys.argv)

    removeSubCases = True
    if 'nopresent' in sys.argv:
        removeSubCases = False
        Paths().skipPresentation = True
    if not 'norun' in sys.argv:
        Suite.run(cleanup=removeSubCases)
    Suite.present()
