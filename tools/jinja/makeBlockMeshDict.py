#!/usr/bin/python
import os,sys
import numpy as np
from jinja2 import Template
from blockMesh import Blocks,Block2D,strList

def genVerts():
    # Generate vertex coordinates, somehow
    # this is for a wedge mesh
    alpha = np.radians(2.5)
    
    x0 = 0
    x1 = 1
    x2 = 1.1
    x3 = 3
    
    y0 = 0
    y1 = 0.1
    y2 = 1
    
    z0 = 0
    z1 = 0.1
    
    dx = 0.01;
    dy = 0.01;
    
    xPos = [x0,x1,x2,x3]
    yPos = [y0,y1,y2]
    zPos = [z0,z1]
    
    nPoints = len(xPos)*len(yPos)
    
    points = np.zeros(2*nPoints*3)
    points.shape = (2*nPoints,3)
    count = 0
    for y in yPos:
        for x in xPos:
            points[count] = [x,y,-y*np.tan(alpha)]
            points[count+nPoints] = [x,y,y*np.tan(alpha)]
            count += 1
    return points

def writeAndCreateMesh(blockMeshDict):
    from subprocess import Popen,PIPE
    dictPath = 'system/blockMeshDict_tmp'
    with open(dictPath,'w') as dict:
        dict.write(blockMeshDict)
    cmd = ['blockMesh','-dict',dictPath]
    p = Popen(cmd,stdout=PIPE,stderr=PIPE)
    out,err = p.communicate()
    for line in out.split('\n'.encode()):
        print(line.decode())
    if err:
        for line in err.split('\n'.encode()):
            print('Error:',line.decode())
 

def run():
    # Global block parameters
    Block2D.verts = genVerts()
    Block2D.cellSize = 0.005
    
    # Define blocks
    b0 = Block2D([0,1,5,4])
    b0.grading = [0.2,0.2,1]
    b0.addBoundary('x0', 'inflow')
    b0.addBoundary('z0', 'wedge0')
    b0.addBoundary('z1', 'wedge1')
    b0.addBoundary('y0', 'axis')
    
    b1 = Block2D([1,2,6,5])
    b1.grading = [1,0.2,1]
    b1.res[0] *= 2
    b1.addBoundary('y1', 'walls')
    b1.addBoundary('z0', 'wedge0')
    b1.addBoundary('z1', 'wedge1')
    b1.addBoundary('y0', 'axis')
    
    b2 = Block2D([2,3,7,6])
    b2.grading = [5,0.2,1]
    b2.addBoundary('x1', 'outflow')
    b2.addBoundary('z0', 'wedge0')
    b2.addBoundary('z1', 'wedge1')
    b2.addBoundary('y0', 'axis')
    
    b3 = Block2D([4,5,9,8])
    b3.grading = [0.2,5,1]
    b3.addBoundary('x0', 'inflow')
    b3.addBoundary('x1', 'walls')
    b3.addBoundary('y1', 'walls')
    b3.addBoundary('z0', 'wedge0')
    b3.addBoundary('z1', 'wedge1')
    
    b4 = Block2D([6,7,11,10])
    b4.grading = [5,5,1]
    b4.addBoundary('x0', 'walls')
    b4.addBoundary('x1', 'outflow')
    b4.addBoundary('y1', 'walls')
    b4.addBoundary('z0', 'wedge0')
    b4.addBoundary('z1', 'wedge1')
    
    B = Blocks([b0,b1,b2,b3,b4])
    B.makePatch('walls','wall')
    B.makePatch('inflow','patch')
    B.makePatch('outflow','patch')
    B.makePatch('wedge0','wedge')
    B.makePatch('wedge1','wedge')
    B.makePatch('axis','empty')
    
    template = open('blockMeshDictTemplate.jinja').read()
    
    t = Template(template)
    blockMeshDict = t.render(blocks=B.blockDefs(),
                             vertices=(strList(p) for p in Block2D.verts),
                             patches=B.boundaryDefs())
    
    if os.path.isdir('system'):
        writeAndCreateMesh(blockMeshDict)
    else:
        print(blockMeshDict)

if __name__ == '__main__':
    run()
