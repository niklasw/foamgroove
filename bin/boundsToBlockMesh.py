#!/usr/bin/env python


class BoundingBlockDict:

    def __init__(self,bounds=([],[]),scale = 1.0,size=0.05):
        self.rez = [1,1,1]
        self.block = range(8)
        self.scale = scale
        self.size = size
        self.bounds = bounds

    def interactor(self):
        from interactor2 import interactor,ioList
        i = interactor()
        minb = i.get(prompt='Box min corner coordinate', default='0 0 0', test=ioList)
        maxb = i.get(prompt='Box max corner coordinate', default='0 0 0', test=ioList)
        self.scale = i.get(prompt='Scale [m]', default=1.0,test=float)
        self.size = i.get(prompt='Base size [m]', default=0.05,test=float)
        self.infoRefinementLevels(self.size,1,8)
        self.output = i.get(test=str,default='constant/polyMesh/blockMeshDict')
        self.bounds=(minb,maxb)

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

    def createBlock(self, name='', offsetFactor=0.0):
        from numpy import asarray,array

        min,max = self.bounds
        max = asarray(max)
        min = asarray(min)
        # Extend the bounding box by 5% (ad hoc) in each direction
        max += abs(max)*offsetFactor
        min -= abs(min)*offsetFactor
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
            print "Argument offsetFactor must be float"
    print "Offset factor set to ",offFactor

    B = BoundingBlockDict()
    B.interactor()
    B.createBlock(offsetFactor=offFactor)
    print B
    B.write()
