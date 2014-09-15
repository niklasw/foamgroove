rm -rf [1-9]* *e-[0-9] 0.*
foamClearPolyMesh
blockMesh -dict system/blockMeshDict
changeDictionaryLight
transformPoints -rotate "((0 0 1) (1 0 0))"
topoSet
