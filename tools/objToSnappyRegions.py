#!/usr/bin/env python

import os,sys,re

def getObjGroups(objFile):
    pat = re.compile(r'^g\s+(\w*)\s*')
    groups = []
    with open(objFile,'r') as fp:
        for line in fp:
            match = pat.match(line)
            if match:
                groups.append(match.group(1))
    return groups

class SnappyDict:
    def __init__(self, objFileName=''):
        self.fileName = objFileName

    def interactor(self):
        from interactor2 import interactor
        i = interactor()
        self.fileName, name = i.iFileSelector(path='constant/triSurface',suffix='.obj*')
        self.output = i.get(test=str,default='system/snappyHexMeshDict')

    def write(self):
        with open(self.output,'w') as fp:
            fp.write(self.__str__())

    def patchInfo(self,name):
        import re
        typeMap = {'wall':'wall',
                   'inlet':'patch',
                   'outlet': 'patch'}
        for key,value in typeMap.items():
            pat = re.compile(".*%s.*"%key,re.IGNORECASE)
            if pat.match(name):
                return '%sGroup' % key, value
                break
        return 'wallGroup','wall'


    def composeRegions(self):
        from fgDictTemplates import dbDict

        objGroups = getObjGroups(self.fileName)

        objName = os.path.basename(self.fileName)

        featuresFile = os.path.splitext(objName)[0]+'.eMesh'

        regionString = ''
        refString = ''
        levelString = ''

        e = dbDict()

        # Build region information string

        e.ilevel(1)
        e.new(objName,subDict=False)
        e.addEntry('type','triSurfaceMesh')
        e.new('regions',subDict=True)
        for solid in objGroups:
            e.new(solid,subDict=True)
            e.addEntry('name',solid)
            e.finish()
        e.finish()
        e.finish()
        e.ilevel(-1)

        regionString += e.flush()

        # build refnement regions information string
        e.ilevel(2)
        e.new(objName,subDict=False)
        e.addEntry('level','(0 0)')
        e.new('regions',subDict=True)
        for solid in objGroups:
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

