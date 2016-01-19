#!/usr/bin/env python

import os,sys,re,string
from struct import *
from types import *


def getArgs():
    from optparse import OptionParser
    descString = """
    Python thing to scale stl file vertices
    """

    parser=OptionParser(description=descString)
    parser.add_option('-f','--file',dest='stlFile',default=None,help='Input stl file')
    parser.add_option('-i','--inplace',dest='inplace',default=False,help='NOT IMPLEMENTED! Replace input file')
    parser.add_option('-o','--output',dest='output',default=None,help='Output stl file')
    parser.add_option('-s','--scale',dest='scale',default=1.0,help='Scaling factor')
    parser.add_option('-B','--boundingbox',dest='bounds',action='store_true',default=False,help='Calculate bounding box')
    parser.add_option('-t','--split',dest='split',action='store_true',default=False,help='Split into separate files')
    parser.add_option('-b','--binary',dest='binary',action='store_true',default=False,help='Input file is binary')
    parser.add_option('-T','--translate',dest='translate',default=None,help='Translate all vertices')

    (opt,arg)=parser.parse_args()

    def argError(s):
        s = '* ERROR: %s. *' % s
        n=len(s)
        print '\n\t%s\n\t%s\n\t%s\n' % (n*'*',s,n*'*')
        parser.print_help()
        sys.exit(1)

    if not opt.stlFile: argError( 'Missing stl file argument' )
    stlIn = opt.stlFile
    stlBase = os.path.splitext(stlIn)[0]
    stlExt = os.path.splitext(stlIn)[1]
    stlOut = opt.output
    if not opt.output:
        stlOut = stlBase+'_scaled'+stlExt
    translation=[0,0,0]
    if opt.translate:
        tString = opt.translate
        try:
            translation = map(float,opt.translate.strip('()').split())
        except:
            argError('translation vector not in format \"(x y z)\".')



    argDict = { 'stlIn':stlIn,
                'stlOut':stlOut,
                'scale':float(opt.scale),
                'split':opt.split,
                'bounds':opt.bounds,
                'translate':translation,
                'binary':opt.binary}

    return argDict


class stlToolBox:
    def __init__(self,stlFile,stlOutFile,binary=False):
        self.stlFile = stlFile
        readOpt = 'r'
        if binary: readOpt='rb'
        self.stlHandle = self.openFile(readOpt)
        self.vertPat = re.compile('^\s*vertex\s+',re.I)
        self.regions = []
        self.stlOut = stlOutFile
        self.verts = []
        self.binary = binary

    def openFile(self,readOpt):
        import gzip
        def isGzip(f):
            try:
                s=open(f,'rb').read(2)
                s=s.encode('hex')
                return (s == '1f8b')
            except:
                print 'Cannot open stl file'
        f = gzip.open if isGzip(self.stlFile) else open
        return f(self.stlFile,readOpt)

    def action(self, scale=1.0, split=False, bounds=False, translate=False,binary=False):
        if sum(map(abs,translate)) > 0:
            self.translate(translate)
            sys.exit(0)
        if bounds:
            self.calcBounds()
            sys.exit(0)
        if scale != 1.0:
            self.readVerts(binary)
            #self.scaleV(scale)
            self.scale(scale)
            sys.exit(0)
        if split:
            self.split()
            sys.exit(0)

    def readVerts(self,binary=False):
        print 'Reading stl'
        from pylab import array,zeros
        self.stlHandle.seek(0)
        if not binary:
            print 'Ascii input'
            for line in self.stlHandle:
                line = line.strip()
                if line[0] == 'v':
                    print line
                    v = array(map(float,line.split()[-3:]))
                    self.verts.append(v)
        else:
            print 'Binary input'
            header = unpack("<80s", self.stlHandle.read(80))
            print header
            facets = unpack("i", self.stlHandle.read(4))
            v=zeros(3)
            for i in range(0, facets[0]):
                t = self.stlHandle.read(12)
                for i in range(3):
                    for j in range(3):
                        v[j] = unpack("f", self.stlHandle.read(4))[0]
                    self.verts.append(v)

    def scaleV(self,scale):
        print 'Scaling stl file by factor',scale
        outFp = open(self.stlOut,'w')
        vertStr = 'vertex'
        for vert in self.verts:
            vert *= scale
            line = '\tvertex %f %f %f\n' % (vert[0],vert[1],vert[2])
            outFp.write(line)
        outFp.close()

    def translate(self,translate):
        from pylab import array
        print 'Translating stl geometry by vector',translate
        outFp = open(self.stlOut,'w')
        self.stlHandle.seek(0)
        for line in self.stlHandle:
            if self.vertPat.match(line):
                temp,x,y,z = line.split()
                xyz = array(map(float,[x,y,z]))
                translate = array(translate)
                X,Y,Z = translate + xyz
                line = string.join(map(str,['\t'+temp,X,Y,Z]))+'\n'
            outFp.write(line)
        outFp.close()

    def scale(self,scale):
        print 'Scaling stl file by factor',scale
        outFp = open(self.stlOut,'w')
        self.stlHandle.seek(0)
        stlLines = self.stlHandle.readlines()
        for line in stlLines:
            if self.vertPat.match(line):
                temp,x,y,z = line.split()
                X,Y,Z = [ scale * a for a in map(float,[x,y,z]) ]
                line = string.join(map(str,['\t'+temp,X,Y,Z]))+'\n'
            outFp.write(line)
        outFp.close()

    def calcBounds(self, name=''):
        print 'Calculating bounding box for', self.stlFile
        minX=minY=minZ =  1.0e10
        maxX=maxY=maxZ = -1.0e10
        stlLines = []
        self.stlHandle.seek(0)
        stlLines = self.stlHandle.readlines()
        #if not name:
        #    self.stlHandle.seek(0)
        #    stlLines = self.stlHandle.readlines()
        #else:
        #    stlLines = self.readSolid(name)

        cX=cY=cZ=0.0
        for line in stlLines:
            if self.vertPat.match(line):
                temp,x,y,z = line.split()
                X,Y,Z = map(float,[x,y,z])
                minX,minY,minZ = min(minX,X),min(minY,Y),min(minZ,Z)
                maxX,maxY,maxZ = max(maxX,X),max(maxY,Y),max(maxZ,Z)
                cX,cY,cZ=[(maxX+minX)/2.,(maxY+minY)/2.,(maxZ+minZ)/2.]
        print '''
        STL bounds for %s =
        centre (%f, %f, %f)
        min  (%f, %f, %f)
        max  (%f, %f, %f)
        size (%f, %f, %f)
        ''' % (self.stlFile,cX,cY,cZ,minX,minY,minZ,maxX,maxY,maxZ,maxX-minX,maxY-minY,maxZ-minZ)

        return (minX,minY,minZ),(maxX,maxY,maxZ)

    def readSolid(self,name=''):

        def readToEnd(endPat,lines=[]):
            line = 'XXX#'
            while line:
                line = self.stlHandle.readline()
                lines.append(line)
                match = endPat.match(line)
                if match:
                    break
            return lines

        begin = re.compile('^\s*solid (.*)$')
        if name:
            begin = re.compile('^\s*solid '+name+'\s*$')
        end = re.compile('^\s*endsolid')
        lines = []
        line = 'XXX#'
        while line:
            line = self.stlHandle.readline()
            match = begin.match(line)
            if match:
                lines.append(line)
                name = match.groups()[0]
                lines += readToEnd(end)
                break
        if not lines:
            print 'readSolid(): Could not find a matching solid.'
        return (name, lines)

    def split(self):
        print 'Spliting stl file into regions:'
        name = 'XXX#'
        while name:
            try:
                name, lines = self.readSolid()
                if name:
                    print '-  ',name
                    fout = open(name+'.stl','w')
                    [ fout.write(line) for line in lines ]
                    fout.close()
            except:
                break

def getStlSolids(stlFile):
    import re
    tool = stlToolBox(stlFile,'None',False)
    fp = tool.openFile('r')

    solids = []
    solidPat = re.compile('^\s*solid (.*)$')
    for line in fp:
        match = solidPat.match(line)
        if match:
            solids.append(match.groups()[0])
    return solids


if __name__=='__main__':
    arguments = getArgs()
    tool = stlToolBox(arguments['stlIn'],arguments['stlOut'],binary=arguments['binary'])
    tool.action(
            bounds = arguments['bounds'],
            scale = arguments['scale'],
            split = arguments['split'],
            translate = arguments['translate'],
            binary = arguments['binary'])

    print '\n---------------- END ----------------'


