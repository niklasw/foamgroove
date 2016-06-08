#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os,sys,glob,re
from suiteTools import SuiteRunner
from testSuiteUtils import *

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

import signal
def signal_handler(signal, frame):
    print('Killed by SIGINT.')
    print('Manually clean out stale subCases')
    sys.exit(0)

if __name__=="__main__":

    if not len(sys.argv) > 1:
        Error('Path to tests top level as sole argument needed')

    signal.signal(signal.SIGINT, signal_handler)

    paths = BorgPaths(testRoot = os.getcwd(), \
                      presentRoot = '/tmp/testSuite/results/', \
                      appRoot = os.path.dirname(sys.argv[0]))

    Suite = SuiteRunner(sys.argv)
    if not 'norun' in sys.argv:
        Suite.run()
    Suite.present()
