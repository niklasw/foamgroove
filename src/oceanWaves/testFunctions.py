#!/usr/bin/python

from pylab import *

def waveNumber(wl):
    return 2*pi/wl

def celerity(wl,wd,g):
    return sqrt(g*wl/(2*pi)*tanh(2*pi*wd/wl))

def omega(c,k):
    return c*k

def period(c,k):
    return 2*pi/omega(c,k)

def sqr(a):
    return a**2

def elevation(wh,wl,wd,t):
    A = wh/2
    k = waveNumber(wl)
    kh = k*wd
    c = celerity(wl,wd,g)
    w = omega(c,k)

    h = A*cos(w*t)                                \
        + sqr(A)*k*cosh(kh)/(4*pow(sinh(2*kh),3)) \
        * (2+cosh(2*kh))*cos(2*(w*t))

    h1= A*sin(w*t)                                \
        + sqr(A)*k*sinh(kh)/(4*pow(cosh(2*kh),3)) \
        * (2+sinh(2*kh))*sin(2*(w*t))

    sigma = tanh(kh)
    h2= A*(sin(w*t)+k*A*(3-sigma**2)/(4*sigma**3)*sin(2*w*t))


    return h,h1,h2


wh = 0.2
wl = 2.0
wd = 2.2
g  = 9.81

c = celerity(wl,wd,g)
k = waveNumber(wl)
print 'celerity = {0}\nperiod = {1}'.format(c, period(c,k))

T = arange(0,pi/2,0.01)

h,h1,h2 = elevation(wh,wl,wd,T)

plot(T,h)
plot(T,h1)
plot(T,h2)
show()
