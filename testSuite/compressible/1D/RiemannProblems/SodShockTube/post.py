#!/usr/bin/env pvpython
# -*- coding: utf-8 -*-

import re,sys,os,string
from postTools import PostTool

caseRoot = os.getcwd()
pvt = None

poster  = PostTool(caseRoot,pvt)

figName='density'
description='Density'
dataFile='postProcessing/sampleDict/$__endtime/lineX1_rho_p_T.xy'
poster.createGraph(figName,description,dataFile,0,1)

figName='pressure'
description='Pressure'
dataFile='postProcessing/sampleDict/$__endtime/lineX1_rho_p_T.xy'
poster.createGraph(figName,description,dataFile,0,2)

figName='temperature'
description='Temperature'
dataFile='postProcessing/sampleDict/$__endtime/lineX1_rho_p_T.xy'
poster.createGraph(figName,description,dataFile,0,3)

figName='velocity'
description='Velocity'
dataFile='postProcessing/sampleDict/$__endtime/lineX1_U.xy'
poster.createGraph(figName,description,dataFile,0,1)

poster.close()




