#!/usr/bin/env python

from stlTool import stlToolBox

class BoundingBlockDict:

    def __init__(self,fileName='',scale = 1.0,size=0.05, offset=0.0):
        self.fileName = fileName
        self.rez = [1,1,1]
        self.block = range(8)
        self.scale = scale
        self.size = size
        self.offset=offset

    def interactor(self):
        from interactor2 import interactor
        i = interactor()
        self.fileName, name = i.iFileSelector(path='constant/triSurface',suffix='.stl*')
        self.scale = i.get(prompt='Scale [m]', default=1.0,test=float)
        self.size = i.get(prompt='Base size [m]', default=0.05,test=float)
        self.offset = i.get(prompt='Offset factor', default=0.05,test=float)
        self.infoRefinementLevels(self.size,1,8)
        self.output = i.get(test=str,default='constant/polyMesh/blockMeshDict')

    def infoRefinementLevels(self,size,level,maxLevel=8):
        if level >= maxLevel:
            print '-'*20+'\nPress Ctrl C to abort'
            return
        size = size/2.0
        print 'Cell size at level %i = %f mm' % (level, size*1000)
        size = self.infoRefinementLevels(size,level+1,maxLevel)
        return size

    def __str__(self):
        from fgDictTemplates import blockMesh
        return blockMesh().substitute(vertices = self.vertsToStr(), resolution = self.blockRezToStr(), scale=self.scale)

    def write(self):
        with open(self.output,'w') as fp:
            fp.write(self.__str__())

    def createBlock(self, name=''):
        from numpy import asarray,array

        tool = stlToolBox(self.fileName,'',False)

        min,max = tool.calcBounds()
        max = asarray(max)
        min = asarray(min)
        # Extend the bounding box by 5% (ad hoc) in each direction
        max += abs(max)*self.offset
        min -= abs(min)*self.offset
        diagonal = max - min

        block = self.block
        block[0] = (min[0],min[1],min[2])
        block[1] = (max[0],min[1],min[2])
        block[2] = (max[0],max[1],min[2])
        block[3] = (min[0],max[1],min[2])

        block[4] = (min[0],min[1],max[2])
        block[5] = (max[0],min[1],max[2])
        block[6] = (max[0],max[1],max[2])
        block[7] = (min[0],max[1],max[2])

        self.rez = map(int,diagonal/self.size)

    def vertsToStr(self):
        verts = ''
        for v in self.block:
            verts += '\t( %f %f %f )\n' % (v[0],v[1],v[2])
        return verts

    def blockRezToStr(self):
        return '(%i %i %i)' % tuple(self.rez)

if __name__=='__main__':
    import sys
    offFactor = 0.05
    if len(sys.argv) > 1:
        try:
            offFactor = float(sys.argv[1])
        except:
            print "Argument offset must be float"
    print "Offset factor set to ",offFactor

    B = BoundingBlockDict(offset=offFactor)
    B.interactor()
    B.createBlock()
    print B
    B.write()
