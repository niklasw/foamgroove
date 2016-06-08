#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os,sys,glob,re
from suiteTools import SuiteRunner

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

    Suite = SuiteRunner(sys.argv)
    if not 'norun' in sys.argv:
        Suite.run()
    Suite.present()

    #signal.pause()
