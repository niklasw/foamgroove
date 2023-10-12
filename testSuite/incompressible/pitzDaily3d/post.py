#!/usr/bin/env pvpython
# -*- coding: utf-8 -*-

import re,sys,os,string
from pvTools import SliceDef,PvTools
from postTools import PostTool

caseRoot = os.getcwd()
pvt = PvTools('case.foam')
poster  = PostTool(caseRoot,pvt)
sliceSetup = SliceDef()


sliceSetup.SCALAR_FIELD    = 'k'
sliceSetup.FIELD_FUNCTION  = 'mag(U)'
sliceSetup.SCALAR_MIN      = 0
sliceSetup.SCALAR_MAX      = 1
sliceSetup.NORMAL          = (0,0,1)
sliceSetup.ORIGIN          = (0.0,0.0,0.0)
sliceSetup.CAMERA_UP       = (0,1,0)
sliceSetup.CAMERA_POS      = (0.14,0.0,1)
sliceSetup.CAMERA_FOCUS    =  (0.14,0.0,0)
sliceSetup.CAMERA_ZOOM     = 16
sliceSetup.IMAGE_SCALE     = 1
sliceSetup.VIEW_SIZE       = [2500,800]
sliceSetup.LEGEND          = True
sliceSetup.FIELDMODE       = 'CELLS'
sliceSetup.CASE_TYPE       = 'Reconstructed Case'

figName = 'testResult_sqrt_k'
sliceSetup.FIELD_FUNCTION = 'sqrt(k)'
sliceSetup.SCALAR_MIN = 0
sliceSetup.SCALAR_MAX = 2
poster.createSliceImage(sliceSetup,figName,'sqrt(k)')

figName = 'testResult_nut'
sliceSetup.FIELD_FUNCTION = 'nut'
sliceSetup.SCALAR_MIN = 0
sliceSetup.SCALAR_MAX = 2.5e-4
poster.createSliceImage(sliceSetup,figName,'Turbulent viscosity')

fName ='forces'
desc = 'Viscous force on lower wall'
dFile = 'postProcessing/forces/0/forces.dat'
poster.createGraph(fName,desc,dFile,0,4)

fName ='kSample'
desc = 'k sampled along horisontal centerline'
dFile = 'postProcessing/sampleDict/$__endtime/horisontal_k_nut.xy'
poster.createGraph(fName,desc,dFile,0,3)

fName ='nutSample'
desc = 'nut sampled along horisontal centerline'
dFile = 'postProcessing/sampleDict/$__endtime/horisontal_k_nut.xy'
poster.createGraph(fName,desc,dFile,0,4)

dFile = 'postProcessing/sampleDict/$__endtime/horisontal_nut.xy'
poster.createGraph(fName,desc,dFile,0,3)

fName ='nutSampleTopWall'
desc = 'nut sampled along top wall'
dFile = 'postProcessing/sampleDict/$__endtime/horisontalTop_k_nut.xy'
poster.createGraph(fName,desc,dFile,0,4)

dFile = 'postProcessing/sampleDict/$__endtime/horisontalTop_nut.xy'
poster.createGraph(fName,desc,dFile,0,3)

fName ='kSampleTopWall'
desc = 'k sampled along horisontal top wall'
dFile = 'postProcessing/sampleDict/$__endtime/horisontalTop_k_nut.xy'
poster.createGraph(fName,desc,dFile,0,3)

dFile = 'postProcessing/sampleDict/$__endtime/horisontalTop_k.xy'
poster.createGraph(fName,desc,dFile,0,3)

poster.close()

