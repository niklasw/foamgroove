#!/usr/bin/env python

from stlToBoundingBlockMesh import BoundingBlockDict

B = BoundingBlockDict()
B.interactor()

from stlToSnappyRegions import SnappyDict
S = SnappyDict()
S.interactor()

B.createBlock()
B.write()
S.write()

print '\nDone. Now you should run \nblockMesh\nsurfaceFeatureExtract -includedAngle 90 %s abc\nsnappyHexMesh -overwrite\n' % (B.fileName)

