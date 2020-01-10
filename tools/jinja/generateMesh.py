#!/usr/bin/python
import pylab as py
from jinja2 import Template

points_add = 10
points_in_axis = [0, 1, 2, 3, 4]

bc = {}
bc["outlet"]        = ["type patch"]
bc["inlet0"]         = ["type patch"]
bc["inlet1"]         = ["type patch"]
bc["top_wall"]        = ["type wall"]
bc["axis"]        = ["type dummy???"]
bc["wedge0"]  = ["type wedge"]
bc["wedge1"]  = ["type wedge"]

x = []
x.append([0.0, 10.0, 15.0, 30.0, 33.0])
x.append([0.0, 10.0, 15.0, 30.0, 33.0])
x.append([0.0, 10.0, 15.0, 30.0, 33.0])

y = []
y.append([0.0, 5.0, 10.0])
y.append([0.0, 4.0, 10.0])
y.append([0.0, 3.0, 7.0])
y.append([0.0, 3.0, 7.0])
y.append([0.0, 3.5, 8.0])

vertices_def = []
for j, x_vec in enumerate(x):
  for i, xi in enumerate(x_vec):
    vertices_def.append( [xi, y[i][j]])

vertices = []

nx0 = 20
dx0 = (x[0][1]-x[0][0])/nx0

nx1 = int(round((x[0][2]-x[0][1])/dx0))
nx2 = int(round((x[0][3]-x[0][2])/dx0))
nx3 = int(round((x[0][4]-x[0][3])/dx0))

nx4 = nx0
nx5 = nx1
nx6 = nx2
nx7 = nx3

ny0 = int(round((y[0][1]-y[0][0])/dx0))
ny4 = int(round((y[0][2]-y[0][1])/dx0))

wedge_alpha = 7.0*py.pi/180

for l0 in vertices_def:
  l = list(l0)
  z0 = l[1]*py.sin(wedge_alpha)
  l.append(-z0)
  vertices.append(l)

for l0 in vertices_def:
  if l0[1] > 1e-8: # Since wedge, remove "collapsed" points at axis.
    l = list(l0)
    z0 = l[1]*py.sin(wedge_alpha)
    l.append(z0)
    vertices.append(l)

vertices_str = []
for l in vertices:
  vertices_str.append(' '.join([str(x) for x in l]))

block_def = [
#0
    ([0,1,6,5],(1,1,0,0),("inlet0", "axis", None, None), [nx0, ny0, 1]),
#1
    ([1,2,7,6],(0,1,0,0),(None, "axis", None, None), [nx1, ny0, 1]),
#2
    ([2,3,8,7],(0,1,0,0),(None, "axis", None, None), [nx2, ny0, 1]),
#3
    ([3,4,9,8],(0,1,1,0),(None, "axis", "outlet", None), [nx3, ny0, 1]),
#4
    ([5,6,11,10],(1,0,0,1),("inlet1", None, None, "top_wall"), [nx4, ny4, 1]),
#5
    ([6,7,12,11],(0,0,0,1),(None, None, None, "top_wall"), [nx5, ny4, 1]),
#6
    ([7,8,13,12],(0,0,0,1),(None, None, "outlet", "top_wall"), [nx6, ny4, 1]),
#7
    ([8,9,14,13],(0,0,1,1),(None, None, "outlet", "top_wall"), [nx7, ny4, 1]),
]

blocks = []
z_points = {}

# Create blocks.
for t0 in block_def:
  a = t0[0][:]
  for x in t0[0]:
    if x in points_in_axis:
      tmp = 0
    else:
      tmp = points_add
    z_points[x] = x+tmp
    a.append(x+tmp)
  blocks.append([a, t0[3]])

blocks_str = []
#for l in blocks:
#  blocks_str.append(' '.join([str(x) for x in l]))

for l in blocks:
  blocks_str.append([' '.join([str(x) for x in l[0]]),
                     ' '.join([str(x) for x in l[1]])])

faces = {}
for key in bc:
  faces[key] = []

# Create faces. Wedges treated separately.
for t0 in block_def:
  for i,s in enumerate(t0[1]):
    if s != 0:
      if s == 1:
        f = [t0[0][i-1], t0[0][i], 
            z_points[t0[0][i]], z_points[t0[0][i-1]]]
        faces[t0[2][i]].append(f)
      if s == -1:
        f = [t0[0][i], t0[0][i-1], 
            z_points[t0[0][i-1]], z_points[t0[0][i]]]
        faces[t0[2][i]].append(f)

# Wedges below.
for t0 in block_def:
   l0 = []
   l0[:] = t0[0]
   l0.reverse()
   faces["wedge0"].append(l0)

   l0 = [z_points[x] for x in t0[0]]
   faces["wedge1"].append(l0)

faces_str = {}
for k, l0 in faces.items():
  faces_str[k] = []
  for l in l0:
    faces_str[k].append(' '.join([str(x) for x in l]))

content = open('blockMeshDictTempl').read()
t = Template(content)
t_str = t.render(blocks = blocks_str, vertices = vertices_str, bc = bc, faces =
        faces_str)
print(t_str)
