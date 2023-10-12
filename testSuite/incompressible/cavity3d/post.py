#!/usr/bin/env pvpython
# -*- coding: utf-8 -*-

import re,sys,os,string
from pvTools import SliceDef,PvTools
from postTools import PostTool

caseRoot = os.getcwd()
# If no paraview operations are made, set pvt to None instead:
pvt     = PvTools('case.foam')
#pvt = None

poster  = PostTool(caseRoot,pvt)

# Begin paraview stuff #############################################
if pvt is not None:
    sliceSetup = SliceDef()
    
    
    sliceSetup.SCALAR_FIELD    = 'k'
    sliceSetup.FIELD_FUNCTION  = 'mag(U)'
    sliceSetup.SCALAR_MIN      = 0
    sliceSetup.SCALAR_MAX      = 1
    sliceSetup.NORMAL          = (0,0,1)
    sliceSetup.ORIGIN          = (0.05,0.05,0.005)
    sliceSetup.CAMERA_UP       = (0,1,0)
    sliceSetup.CAMERA_POS      = (0.05,0.05,0.3)
    sliceSetup.CAMERA_FOCUS    =  (0.05,0.05,0.1)
    sliceSetup.CAMERA_ZOOM     = 16
    sliceSetup.IMAGE_SCALE     = 1
    sliceSetup.VIEW_SIZE       = [1000,1000]
    sliceSetup.LEGEND          = True
    sliceSetup.FIELDMODE       = 'CELLS'
    sliceSetup.CASE_TYPE       = 'Reconstructed Case'
    
    figName = 'testResult_magU'
    sliceSetup.FIELD_FUNCTION = 'mag(U)'
    sliceSetup.SCALAR_MAX = 1 
    poster.createSliceImage(sliceSetup,figName,'mag(U)')
    
    figName = 'testResult_sqrt_k'
    sliceSetup.FIELD_FUNCTION = 'sqrt(k)'
    sliceSetup.SCALAR_MAX = 0.5
    poster.createSliceImage(sliceSetup,figName,'sqrt(k)')
    
    figName = 'testResult_nut'
    sliceSetup.FIELD_FUNCTION = 'nut'
    sliceSetup.SCALAR_MAX = 1e-5
    poster.createSliceImage(sliceSetup,figName,'Turbulent viscosity')
# End paraview stuff    #############################################

figName='forces'
description = 'Viscous force on moving wall'
dataFile='postProcessing/forces/0/forces.dat'
poster.createGraph(figName,description,dataFile,0,4)

figName='UySample'
description='Uy sampled along horisontal centerline'
dataFile = 'postProcessing/sampleDict/$__endtime/horisontal_U.xy'
poster.createGraph(figName,description,dataFile,0,4)

figName='UxSample'
description='Ux sampled along vertical centerline'
dataFile = 'postProcessing/sampleDict/$__endtime/vertical_U.xy'
poster.createGraph(figName,description,dataFile,1,3)

figName='nutSample'
description='Nut sampled along horizontal centerline'
dataFile = 'postProcessing/sampleDict/$__endtime/horisontal_k_nut.xy'
poster.createGraph(figName,description,dataFile,0,4)
# If above dataFile not exist, try with another name
dataFile = 'postProcessing/sampleDict/$__endtime/horisontal_nut.xy'
poster.createGraph(figName,description,dataFile,0,3)

figName='kTopSample'
description='k sampled along horizontal line close to top '
dataFile = 'postProcessing/sampleDict/$__endtime/topHorisontal_k_nut.xy'
poster.createGraph(figName,description,dataFile,0,3)
# If above dataFile not exist, try with another name
dataFile = 'postProcessing/sampleDict/$__endtime/topHorisontal_nut.xy'
poster.createGraph(figName,description,dataFile,0,3)

poster.close()

# Example of data table presentation below

##tableData = []
##for row in data[20:25]:
##    tableData.append(row[0:4])
##
##table1 = DataTable(tableData,columnNames=['I','Try','a','data table'])
##table1.description = 'A random selection of integrated forces'
##
##tableData2 = copy.deepcopy(tableData)
##tableData2[2] = [9,9,9,9]
##table2 = DataTable(tableData2,columnNames=['I','Try','another','data table'])
##table2.description = 'Another dummy table'
##
##caseBook.dataTables.append(table1)
##caseBook.dataTables.append(table2)



