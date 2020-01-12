#!/usr/bin/python

def strList(l):
    s='('
    for item in l:
        if isinstance(item,list):
            s += '\n    '+ strList(item)
        else:
            s += str(item)+' '
    return s+')'

class Block2D(object):
    verts = []
    cellSize = 0.1
    def __init__(self, labels):
        self.labels = labels
        self.labels.extend([i+int(len(self.verts)/2) for i in labels])
        self.faceMap = {'x0': [0,3,7,4],
                        'x1': [1,5,6,2],
                        'y0': [0,4,5,1],
                        'y1': [3,2,6,7],
                        'z0': [0,1,2,3],
                        'z1': [4,7,6,5]}
        self.boundary ={}
        self.grading = [1,1,1]
        self.res = self.getRes(self.cellSize)

    def addBoundary(self,faceId,name):
        if not name in self.boundary:
            self.boundary[name] = []
        self.boundary[name].append({'id':faceId})

    def getFace(self,faceId):
        return [self.labels[i] for i in self.faceMap[faceId]]

    def getFaces(self,name):
        for face in self.boundary[name]:
            yield self.getFace(face['id'])

    def getRes(self,dx):
        from numpy import array
        bounds = array(self.boundBox())
        sides =  bounds[1] - bounds[0]
        return [int(a) for a in sides[0:2]/dx]+[1]

    def boundBox(self):
        myPoints = [self.verts[i] for i in self.labels]
        x,y,z = zip(*myPoints)
        return [(min(x),min(y),min(z)),
                (max(x),max(y),max(z))]

    def __str__(self):
        return  'hex {} {} simpleGrading {}' \
                .format(strList(self.labels), \
                        strList(self.res),
                        strList(self.grading))

class Blocks:
    def __init__(self,blocks):
        self.blocks = blocks
        self.patchFaces = {}
        self.patchType = {}

    def makePatch(self,name,typ):
        for block in self.blocks:
            if name in block.boundary:
                if not name in self.patchFaces:
                    self.patchFaces[name] = []
                faces = block.getFaces(name)
                self.patchFaces[name].extend(faces)
                self.patchType[name]=typ

    def patchNames(self):
        return self.patchFaces.keys()

    def addBlocks(self,blocks):
        self.blocks.extend(blocks)

    def blockDefs(self):
        for b in self.blocks:
            yield str(b)

    def boundaryDefs(self):
        for n in self.patchNames():
            patchType = self.patchType[n]
            faces = self.patchFaces[n]
            yield [n, patchType, faces]

    def __iter__(self):
        return BlocksIterator(self)


class BlocksIterator:
    def __init__(self,blocks):
        self.Blocks = blocks
        self.iterIndex = 0

    def __next__(self):
        out = self.Blocks.blocks[self.iterIndex]
        self.iterIndex += 1
        if self.iterIndex < len(self.Blocks.blocks):
            return out
        else:
            raise StopIteration


template = \
'''/*--------------------------------*- C++ -*----------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Version:  6
     \\/     M anipulation  |
\*---------------------------------------------------------------------------*/
FoamFile
{
    version     2.0;
    format      ascii;
    class       dictionary;
    object      blockMeshDict;
}
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

convertToMeters 0.01;

vertices
(
{%- for v in vertices %}
    {{ v }}
{%- endfor %}
);

blocks
(
{%- for b in blocks %}
    {{ b }}
{%- endfor %}
);

edges
(
);
boundary
(
{%- for p in patches %}
    {{ p[0] }}
    {
        type {{ p[1] }};
        faces
        (
           {%- for f in p[2] %}
                {{ f }}
           {%- endfor %}
        );
    }
{%- endfor %}
);

mergePatchPairs
(
);

// ************************************************************************* //
'''
