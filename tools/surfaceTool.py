#!/usr/bin/env python

import os,sys,re,string
from struct import *
from types import *


def getArgs():
    from optparse import OptionParser
    descString = """
    Python thing to scale surface file vertices
    """

    parser=OptionParser(description=descString)
    parser.add_option('-f','--file',dest='surfaceFile',default=None,help='Input surface file')
    parser.add_option('-i','--inplace',dest='inplace',default=False,help='NOT IMPLEMENTED! Replace input file')
    parser.add_option('-o','--output',dest='output',default=None,help='Output surface file')
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

    if not opt.surfaceFile: argError( 'Missing surface file argument' )
    surfaceIn = opt.surfaceFile
    surfaceBase = os.path.splitext(surfaceIn)[0]
    surfaceExt = os.path.splitext(surfaceIn)[1]
    surfaceOut = opt.output
    if not opt.output:
        surfaceOut = surfaceBase+'_scaled'+surfaceExt
    translation=[0,0,0]
    if opt.translate:
        tString = opt.translate
        try:
            translation = map(float,opt.translate.strip('()').split())
        except:
            argError('translation vector not in format \"(x y z)\".')



    argDict = { 'surfaceIn':surfaceIn,
                'surfaceOut':surfaceOut,
                'scale':float(opt.scale),
                'split':opt.split,
                'bounds':opt.bounds,
                'translate':translation,
                'binary':opt.binary}

    return argDict

def openFile(fileName,readOpt='r'):
    import gzip
    def isGzip(f):
        try:
            s=open(f,'rb').read(2)
            s=s.encode('hex')
            return (s == '1f8b')
        except:
            print 'Cannot open stl file'
    f = gzip.open if isGzip(fileName) else open
    return f(fileName,readOpt)

class surfaceToolBox:

    def __init__(self,surfaceFile,surfaceOutFile,binary=False):
        self.surfaceFile = surfaceFile
        readOpt = 'r'
        if binary: readOpt='rb'
        self.surfaceHandle = self.openFile(readOpt)
        self.vertPat = re.compile('^\s*vertex\s+',re.I)
        self.regions = []
        self.surfaceOut = surfaceOutFile
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
                print 'Cannot open surface file'
        f = gzip.open if isGzip(self.surfaceFile) else open
        return f(self.surfaceFile,readOpt)

    def action(self, scale=1.0, split=False, bounds=False, translate=False,binary=False):
        if sum(map(abs,translate)) > 0:
            self.translate(translate)
            sys.exit(0)
        if bounds:
            self.calcBounds()
            sys.exit(0)
        if scale != 1.0:
            self.readVerts()
            #self.scaleV(scale)
            self.scale(scale)
            sys.exit(0)
        if split:
            self.split()
            sys.exit(0)

    def scaleV(self,scale):
        print 'Scaling surface file by factor',scale
        outFp = open(self.surfaceOut,'w')
        vertStr = 'vertex'
        for vert in self.verts:
            vert *= scale
            line = '\tvertex %f %f %f\n' % (vert[0],vert[1],vert[2])
            outFp.write(line)
        outFp.close()

    def translate(self,translate):
        from pylab import array
        print 'Translating surface geometry by vector',translate
        outFp = open(self.surfaceOut,'w')
        self.surfaceHandle.seek(0)
        for line in self.surfaceHandle:
            if self.vertPat.match(line):
                temp,x,y,z = line.split()
                xyz = array(map(float,[x,y,z]))
                translate = array(translate)
                X,Y,Z = translate + xyz
                line = string.join(map(str,['\t'+temp,X,Y,Z]))+'\n'
            outFp.write(line)
        outFp.close()

    def scale(self,scale):
        print 'Scaling surface file by factor',scale
        outFp = open(self.surfaceOut,'w')
        self.surfaceHandle.seek(0)
        surfaceLines = self.surfaceHandle.readlines()
        for line in surfaceLines:
            if self.vertPat.match(line):
                temp,x,y,z = line.split()
                X,Y,Z = [ scale * a for a in map(float,[x,y,z]) ]
                line = string.join(map(str,['\t'+temp,X,Y,Z]))+'\n'
            outFp.write(line)
        outFp.close()

    def calcBounds(self, name=''):
        print 'Calculating bounding box for', self.surfaceFile
        minX=minY=minZ =  1.0e10
        maxX=maxY=maxZ = -1.0e10

        cX=cY=cZ=0.0
        for vert in self.yieldVerts():
            X,Y,Z = map(float,vert)
            minX,minY,minZ = min(minX,X),min(minY,Y),min(minZ,Z)
            maxX,maxY,maxZ = max(maxX,X),max(maxY,Y),max(maxZ,Z)
            cX,cY,cZ=[(maxX+minX)/2.,(maxY+minY)/2.,(maxZ+minZ)/2.]
        print '''
        surface bounds for %s =
        centre (%f, %f, %f)
        min  (%f, %f, %f)
        max  (%f, %f, %f)
        size (%f, %f, %f)
        ''' % (self.surfaceFile,cX,cY,cZ,minX,minY,minZ,maxX,maxY,maxZ,maxX-minX,maxY-minY,maxZ-minZ)

        return (minX,minY,minZ),(maxX,maxY,maxZ)


class objToolBox(surfaceToolBox):
    def __init__(self,objFile,objOutFile,binary=False):
        surfaceToolBox.__init__(self,objFile,objOutFile,binary)

    def readVerts(self):
        print 'Reading obj for verts'
        from pylab import array,zeros
        self.surfaceHandle.seek(0)
        if not self.binary:
            print 'Ascii input obj'
            for line in self.surfaceHandle:
                line = line.strip()
                if not line:
                    continue
                if line[0] == 'v':
                    v = array(map(float,line.split()[-3:]))
                    self.verts.append(v)
        else:
            print 'Binary input for obj NOT IMPLEMENTED'
            sys.exit(1)

    def yieldVerts(self):
        print 'Reading obj for verts'
        from pylab import array,zeros
        self.surfaceHandle.seek(0)
        if not self.binary:
            print 'Ascii input obj'
            for line in self.surfaceHandle:
                line = line.strip()
                if not line:
                    continue
                if line[0] == 'v':
                    v = array(map(float,line.split()[-3:]))
                    yield v
        else:
            print 'Binary input for obj NOT IMPLEMENTED'
            sys.exit(1)

    def readSolid(self,name=''):
        print 'objToolBox.split NOT IMPLEMENTED'
        sys.exit(1)

    def split(self):
        print 'objToolBox.split NOT IMPLEMENTED'
        sys.exit(1)


class stlToolBox(surfaceToolBox):
    def __init__(self,stlFile,stlOutFile,binary=False):
        surfaceToolBox.__init__(self,objFile,objOutFile,binary)

    def readVerts(self):
        print 'Reading stl'
        from pylab import array,zeros
        self.stlHandle.seek(0)
        if not self.binary:
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

    def yieldVerts(self):
        from pylab import array,zeros
        self.stlHandle.seek(0)
        if not self.binary:
            print 'Ascii input'
            for line in self.stlHandle:
                line = line.strip()
                if line[0] == 'v':
                    print line
                    v = array(map(float,line.split()[-3:]))
                    yield v
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
                    yield v


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

def getObjSolids(objFile):
    import re
    fp = openFile(objFile)
    solids = []
    solidPat = re.compile('^g(.*)$')
    for line in fp:
        match = solidPat.match(line)
        if match:
            solids.append(match.groups()[0].strip())
    return solids

if __name__=='__main__':
    arguments = getArgs()
    tool = stlToolBox(arguments['surfaceIn'],arguments['surfaceOut'],binary=arguments['binary'])
    tool.action(
            bounds = arguments['bounds'],
            scale = arguments['scale'],
            split = arguments['split'],
            translate = arguments['translate'],
            binary = arguments['binary'])

    print '\n---------------- END ----------------'


