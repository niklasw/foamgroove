#!/usr/bin/env python

import os

class SnappyDict:
    def __init__(self, surfaceFileName=''):
        self.fileName = surfaceFileName

    def interactor(self):
        from interactor2 import interactor
        i = interactor()
        self.fileName, name = i.iFileSelector(path='constant/triSurface')
        self.output = i.get(test=str,default='system/snappyHexMeshDict')

    def write(self):
        with open(self.output,'w') as fp:
            fp.write(self.__str__())

    def patchInfo(self,name):
        import re
        typeMap = {'wall':'wall',
                   'inlet':'patch',
                   'outlet': 'patch'}
        for key,value in typeMap.iteritems():
            pat = re.compile(".*%s.*"%key,re.IGNORECASE)
            if pat.match(name):
                return '%sGroup' % key, value
                break
        return 'wallGroup','wall'

    def getSurfaceSolids(self):
        suffix = os.path.splitext(self.fileName)[1]
        if suffix == '.stl':
            from surfaceTool import getStlSolids
            return getStlSolids
        elif suffix == '.obj':
            from surfaceTool import getObjSolids
            return getObjSolids

    def composeRegions(self):
        from fgDictTemplates import dbDict

        getSolids = self.getSurfaceSolids()

        surfaceSolids = getSolids(self.fileName)

        surfaceName = os.path.basename(self.fileName)

        featuresFile = os.path.splitext(surfaceName)[0]+'.eMesh'

        regionString = ''
        refString = ''
        levelString = ''

        e = dbDict()

        # Build region information string

        e.ilevel(1)
        e.new(surfaceName,subDict=False)
        e.addEntry('type','triSurfaceMesh')
        e.new('regions',subDict=True)
        for solid in surfaceSolids:
            e.new(solid,subDict=True)
            e.addEntry('name',solid)
            e.finish()
        e.finish()
        e.finish()
        e.ilevel(-1)

        regionString += e.flush()

        # build refnement regions information string
        e.ilevel(2)
        e.new(surfaceName,subDict=False)
        e.addEntry('level','(0 0)')
        e.new('regions',subDict=True)
        for solid in surfaceSolids:
            group,ptype = self.patchInfo(solid)
            e.new(solid,level=0,subDict=True)
            e.addEntry('level','(2 3)')
            e.new('patchInfo',subDict=True)
            e.addEntry('type',ptype)
            e.addEntry('inGroups','(%s)'%group)
            e.finish()
            e.finish()
        e.finish()
        e.finish()

        refString    += e.flush()

        return (regionString, refString, featuresFile, levelString)


    def __str__(self):
        from fgDictTemplates import snappyHexMesh

        regionString, refString, emeshFileName, levelString  = self.composeRegions()

        return snappyHexMesh().substitute(geometry=regionString,
                                          refinementSurfaces=refString,
                                          featuresFile=emeshFileName)

if __name__=='__main__':
    S = SnappyDict()
    S.interactor()
    S.write()

