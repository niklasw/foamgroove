#!/usr/bin/env pvpython
# -*- coding: utf-8 -*-

import re,sys,os,string
import matplotlib.pyplot as plt
import numpy
from caseManaging import ResultPicture, Book
from testSuiteUtils import *
print sys.path
sys.path.append('/remote/soft/ParaView/5.0.1/lib/paraview-5.0/lib/python2.7')

caseRoot = os.getcwd()

caseBook = Book.open(caseRoot)

# Create ResultPicture object, to store a picture and its info
picture1 = ResultPicture(root=caseRoot,                           \
                         fileName='forces.png',                        \
                         description = 'Viscous force on moving wall')


# Extract arrays for a plot of integrated force
forcesFile=os.path.join(caseRoot,'postProcessing/forces/0/forces.dat')

dataLines = cleanPostProcessingData(forcesFile)

data = []
for item in dataLines:
    data.append(map(float,item.split()))

t = numpy.array(data)[:,0]
f = numpy.array(data)[:,4]

# Add data to picture, optionally
picture1.data=numpy.array([t,f])

# Create an image and save it to the ResultPicture (not on disk yet)
plt.plot(t,f)
plt.grid('on')
plt.savefig(picture1.path())

caseBook.pictures.append(picture1)
caseBook.close()


