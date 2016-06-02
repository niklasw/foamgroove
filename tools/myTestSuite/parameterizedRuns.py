#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os,sys,glob,re
from SuiteTools import SuiteRunner

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

if __name__=="__main__":

    if not len(sys.argv) > 1:
        Error('Path to tests top level as sole argument needed')

    Suite = SuiteRunner(sys.argv)
    if not 'norun' in sys.argv:
        Suite.run()
    Suite.present()
