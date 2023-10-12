#!/usr/bin/env pvpython
# -*- coding: utf-8 -*-

import re,sys,os,string
from pvTools import SliceDef,PvTools
from postTools import PostTool

caseRoot = os.getcwd()
pvt = PvTools('case.foam')
poster  = PostTool(caseRoot,pvt)

if pvt is not None:
    sliceSetup = SliceDef()


    sliceSetup.SCALAR_FIELD    = 'p'
    sliceSetup.FIELD_FUNCTION  = 'mag(U)'
    sliceSetup.SCALAR_MIN      = 0
    sliceSetup.SCALAR_MAX      = 1
    sliceSetup.NORMAL          = (0,0,1)
    sliceSetup.ORIGIN          = (4,2,1)
    sliceSetup.CAMERA_UP       = (0,1,0)
    sliceSetup.CAMERA_POS      = (4,1.0,4)
    sliceSetup.CAMERA_FOCUS    = (4,2.0,1)
    sliceSetup.CAMERA_ZOOM     = 2
    sliceSetup.IMAGE_SCALE     = 1
    sliceSetup.VIEW_SIZE       = [4000,2000]
    sliceSetup.LEGEND          = False
    sliceSetup.FIELDMODE       = 'CELLS'
    sliceSetup.CASE_TYPE       = 'Reconstructed Case'

    figName = 'UMean'
    function = 'mag(UMean)'
    sliceSetup.FIELD_FUNCTION = function
    sliceSetup.SCALAR_MIN = 0
    sliceSetup.SCALAR_MAX = 0.2
    poster.createSliceImage(sliceSetup,figName,function)

    figName = 'U'
    function = 'mag(U)'
    sliceSetup.FIELD_FUNCTION = function
    sliceSetup.SCALAR_MIN = 0
    sliceSetup.SCALAR_MAX = 1.2
    poster.createSliceImage(sliceSetup,figName,function)

#fName ='USample'
#desc = 'U sampled along horisontal centerline'
#dFile = 'postProcessing/sampleDict/$__endtime/horisontal_U.xy'
#poster.createGraph(fName,desc,dFile,0,3)
#
#fName ='kSample'
#desc = 'k sampled along horisontal centerline'
#dFile = 'postProcessing/sampleDict/$__endtime/horisontal_k.xy'
#poster.createGraph(fName,desc,dFile,0,3)

poster.close()

