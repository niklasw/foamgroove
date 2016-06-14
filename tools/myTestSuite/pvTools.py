#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# Some helper things that can be used in post.py
# 

import os,sys

try:
    try: paraview.simple
    except: from paraview.simple import *
    paraview.simple._DisableFirstRenderCameraReset()
except:
    print 'Paraview API not found'
    sys.exit(1)

class Field:

    def __init__(self,name=None,function=None,colorMin=0,colorMax=0):
        self.name = name
        self.function = function if function else name
        self.min = colorMin
        self.max = colorMax
        self.displayMode = 'CELLS'
        self.functionMode ='Cell Data'

    def setDataMode(self, pointOrCell):
        self.displayMode = pointOrCell
        if pointOrCell == 'CELLS':
            self.functionMode =  'Cell Data'
        elif pointOrCell == 'POINTS':
            self.functionMode = 'Point Data'
        else:
            print 'ERROR: Wrong data mode. Must be CELLS or POINTS'
            sys.exit(1)

    def initFromSetup(self,setup):
        self.name = setup.SCALAR_FIELD
        f = setup.FIELD_FUNCTION
        self.function = f if f else setup.SCALAR_FIELD
        self.min = setup.SCALAR_MIN
        self.max = setup.SCALAR_MAX
        self.setDataMode(setup.FIELDMODE)
        print 'Using field {0} with data mode {1}'.format(self.name, self.displayMode)

class pvTools:

    @staticmethod
    def createVectorRange(startV=(0,0,0), endV=(1,1,1), N=5):
        # Just a pythonic way to create a list of vectors between
        # sweepStart and sweepEnd. I know it looks funny.
        startV = map(float,startV)
        endV = map(float,endV)
        return ([a+n*(b-a)/N for a,b in zip(startV,endV)] for n in range(N+1))

    def __init__(self,dataFile='case.foam', fieldName=None):
        self.dataFile = dataFile
        self.source = self.loadCase()
        self.views = []
        self.currentView = -1 
        self.field = Field(fieldName)
        self.instants = []
        self.figures = []

    def initFromSetup(self,setup):
        self.field = Field()
        self.field.initFromSetup(setup)
        self.initDisplayAndTime(viewSize=setup.VIEW_SIZE)
        self.setCamera(up=setup.CAMERA_UP,pos=setup.CAMERA_POS, \
                       focus=setup.CAMERA_FOCUS,scale=1.0/setup.CAMERA_ZOOM)

    def loadCase(self):
        OpenDataFile(self.dataFile)
        sourcesDict = GetSources()
        source = sourcesDict.values()[0]
        SetActiveSource(source)
        return source

    def getView(self):
        return self.views[self.currentView]

    def setStartTime(self,i=1):
        self.getView().ViewTime = self.instants[i]

    def setLatestTime(self):
        self.getView().ViewTime = self.instants[-1]

    def setTime(self,i):
        self.getView().ViewTime = self.instants[i]

    def saveFig(self,imageBase,mag):
        img = '{0}.png'.format(imageBase)
        print 'Saving image',img
        SaveScreenshot(img,magnification=mag,quality=100)
        self.figures.append(img)

    def animate(self,imageBase, mag):
        for i,t in enumerate(self.instants):
            self.getView().ViewTime = t
            img = '{0}_{1:04d}'.format(imageBase,i)
            self.saveFig(img,mag)

    def updatePipeLine(self, newSource):
        Hide()
        self.source = newSource
        SetActiveSource(self.source)

    def initDisplayAndTime(self,bg=[1,1,1,],viewSize=[2000,1000], \
                           viewName='RenderView'):
        view = GetActiveViewOrCreate(viewName)
        self.views.append(view)
        self.currentView += 1
        SetActiveView(view)
        GetDisplayProperties(view=view)
        self.instants = self.source.TimestepValues
        try:
            self.setLatestTime()
            print 'Time set to latest time: %f' % view.ViewTime
        except:
            pass
        view.UseOffscreenRendering=1
        view.UseOffscreenRenderingForScreenshots=1
        view.Background = bg
        view.ViewSize = viewSize

    def hideDefaultRepresentation(self):
        rep = GetDisplayProperties(self.source)
        rep.Visibility = 0

    def setCamera(self, up=(0,1,0),pos=(1,1,1),focus=(0,0,0), \
                  scale=0.1,parallel=1,clip=(-10,10), mode='2D'):
        view = self.getView()
        view.InteractionMode = mode
        view.CameraViewUp = up
        view.CameraPosition = pos
        view.CameraClippingRange = clip
        view.CameraFocalPoint = focus
        view.CameraParallelProjection = parallel
        view.CameraParallelScale = scale

    def createSlice(self, origin = (0,0,0), normal = (0,1,0)):
        mySlice = Slice(SliceType='Plane')
        mySlice.SliceType.Origin = origin
        mySlice.SliceType.Normal = normal
        self.updatePipeLine(mySlice)
        return mySlice

    def setColorRange(self, colorMin=0, colorMax=0):
        self.field.min = colorMin
        self.field.max = colorMax
        scalarColorLUT = GetColorTransferFunction(self.field.name)
        scalarColorPWF = GetOpacityTransferFunction(self.field.name)
        if not colorMin == colorMax:
            scalarColorLUT.RescaleTransferFunction(self.field.min,self.field.max)
            scalarColorPWF.RescaleTransferFunction(self.field.min,self.field.max)

    def createScalarRepresentation(self, scalarBar=False):
        print 'Using scalar field {0}: min = {1}, max = {2}'.format(
                self.field.name,self.field.min, self.field.max)
        display = GetDisplayProperties(GetActiveSource(),view=self.getView())
        ColorBy(display, (self.field.displayMode,self.field.name))
        display.SetScalarBarVisibility(self.getView(), scalarBar)
        self.setColorRange(self.field.min,self.field.max)
 
    def calculator(self):
        Hide()
        calc = Calculator(Input=self.source)
        calc.AttributeMode = self.field.functionMode
        calc.Function = self.field.function
        calc.ResultArrayName=self.field.function
        self.field.name=self.field.function
        self.updatePipeLine(calc)
        self.createScalarRepresentation()
        return calc

    def legend(self,scalarBar=True):
        GetDisplayProperties().SetScalarBarVisibility(self.getView(),scalarBar)


class plotter:

    def __init__(self,dataPath, instant, figPrefix):
        self.dataPath = dataPath
        self.instant = instant
        self.figPrefix = figPrefix
        self.figures = []

    def isFile(self,fileBaseName):
        filen = os.path.join(self.dataPath,self.instant,fileBaseName)
        if os.path.isfile(filen):
            return filen
        return False


    def plot(self,field,fileBaseName,columns):
        from numpy import loadtxt
        from matplotlib import pyplot as plt
        filen = self.isFile(fileBaseName)
        if not filen:
            return False
        name = fileBaseName.split('_')[0]
        img = '{0}_{1}_{2}.png'.format(self.figPrefix,name,field)

        data = loadtxt(filen)
        x=data[:,columns[0]]
        y=data[:,columns[1]]
        plt.plot(x,y,'r-o')
        plt.grid('on')
        plt.title('{0} {1}'.format(name,field))
        plt.savefig(img)
        self.figures.append(img)
        plt.close()
        return True


if __name__=="__main__":

    class setup:
        SCALAR_FIELD    = 'k'
        FIELD_FUNCTION  = 'sqrt(k)*mag(U)'
        SCALAR_MIN      = 0
        SCALAR_MAX      = 5
        NORMAL          = (0,0,1)
        ORIGIN          = (0,0,0)
        CAMERA_UP       = (0,1,0)
        CAMERA_POS      = (0.13, 0, 0.01)
        CAMERA_FOCUS    = (0.13, 0, 0)
        CAMERA_ZOOM     = 16
        IMAGE_SCALE     = 1
        VIEW_SIZE       = [2000,500]
        LEGEND          = True
        FIELDMODE       = 'CELLS' # 'POINTS'



    pvt = pvTools('case.foam')
    pvt.initFromSetup()
    pvt.source.CaseType = 'Reconstructed Case'

    Hide()
    pvt.createSlice(origin=setup.ORIGIN,normal=setup.NORMAL)
    pvt.calculator()
    pvt.legend(setup.LEGEND)
    Show()

    pvt.saveFig('testFig',setup.IMAGE_SCALE)
    pvt.animate('testAnim',setup.IMAGE_SCALE)

