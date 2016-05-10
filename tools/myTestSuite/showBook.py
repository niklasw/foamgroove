#!/usr/bin/env python

import re,sys,os,string
import matplotlib.pyplot as plt
from caseManaging import ResultPicture, Book
from testSuiteUtils import *

book = Book.open(sys.argv[1])

for key in book.logData.keys():
    print book.logData[key]

for key in book.errData.keys():
    print book.errData[key]

print book.logData.keys()
for picture in book.pictures:
    print picture.htmlPrint()
