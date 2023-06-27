#!/usr/bin/python
# Only for testing different formulations for wave elevation
# based on Stokes second order ocean wave approximation
#

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

    h = A*cos(w*t)                              \
        + sqr(A)*k*cosh(kh)/(4*pow(sinh(kh),3)) \
        * (2+cosh(2*kh))*cos(2*(w*t))

    h1 = A*sin(w*t)                              \
        + sqr(A)*k*cosh(kh)/(4*pow(sinh(kh),3)) \
        * (2+cosh(2*kh))*sin(2*(w*t))

    sigma = tanh(kh)
    h2= A*(cos(w*t)+k*A*(3-sigma**2)/(4*sigma**3)*cos(2*w*t))


    return h,h1,h2


wh = 0.2
wl = 9.0
wd = 10.0
g  = 9.81

c = celerity(wl,wd,g)
k = waveNumber(wl)
print 'celerity = {0}\nperiod = {1}'.format(c, period(c,k))
print 'k*a =a*2pi/L = {0}'.format(wh/2*2*pi/wl)

T = arange(0,period(c,k),0.01)

for wl in arange(1.0,11,1):
    h,h1,h2 = elevation(wh,wl,wd,T)
    plot(T,h2,label=str(wl))
legend()
grid()
show()
