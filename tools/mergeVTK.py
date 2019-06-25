#!/usr/bin/env pvpython

import sys

from vtk import *

reader = vtkPolyDataReader()
append = vtkAppendPolyData()

filenames = sys.argv[1:]
for fil in filenames:
    reader.SetFileName(fil)
    reader.Update()
    polydata = vtkPolyData()
    polydata.ShallowCopy(reader.GetOutput())
    append.AddInputData(polydata)

append.Update()

writer = vtkPolyDataWriter()
writer.SetFileName('output.vtk')
writer.SetInputConnection(append.GetOutputPort())
writer.Write()
