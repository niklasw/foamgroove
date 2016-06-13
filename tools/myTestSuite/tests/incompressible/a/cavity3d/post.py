#!/usr/bin/env pvpython
# -*- coding: utf-8 -*-

import re,sys,os,string
import copy
import matplotlib.pyplot as plt
import numpy
from caseBook import ResultPicture, Book, DataTable
from testSuiteUtils import *

caseRoot = os.getcwd()

caseBook = Book.open(caseRoot)

# Create ResultPicture object, to store a picture and its info
picture1 = ResultPicture(root=caseRoot,                           \
                         fileName='forces.png',                        \
                         description = 'Viscous force on moving wall')

with open('fff','w') as fp:
    fp.write(caseBook.description)


# Extract arrays for a plot of integrated force
forcesFile=os.path.join(caseRoot,'postProcessing/forces/0/forces.dat')

dataLines = cleanPostProcessingData(forcesFile)

data = []
for item in dataLines:
    data.append(map(float,item.split()))

t = numpy.array(data)[:,0]
f = numpy.array(data)[:,4]

# Add data to picture, optionally
picture1.dataSet=numpy.array([t,f])

# Create an image and save it to the ResultPicture (not on disk yet)
plt.plot(t,f)
plt.grid('on')
plt.savefig(picture1.path())

caseBook.pictures.append(picture1)
picture2 = copy.deepcopy(picture1)
picture2.description = 'Another picture, but same'
caseBook.pictures.append(picture2)
caseBook.pictures.append(picture1)

tableData = []
for row in data[20:25]:
    tableData.append(row[0:4])

table = DataTable(tableData,columnNames=['I','Try','a','data table'])
table.description = 'A random selection of integrated forces'

caseBook.dataTables.append(table)

caseBook.close()


