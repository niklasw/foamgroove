#!/usr/bin/python

from pylab import *

t = linspace(0,10,200)

frq = 10

Q = 1

x0 = 0
x1 = 0

w = 2*pi*frq
x =  (Q*(-cos(w*t))+Q+ x1*w*sin(t*w))/w**2

plot(t,x)

grid(True)

show()


