#!/usr/bin/python
from pylab import *
from numpy import *

def filter(p,Nf):
    pf = zeros(size(p))
    N = len(p)
    
    for i in range(N):
        ind1 = i
        ind2 = min(i+Nf-1,N-1)
        nn = ind2 - ind1 + 1
        
        tmp = 0.0
        for j in range(nn):
            tmp = tmp + p[i+j]
        pf[i] = tmp/nn
                
    return (pf)

def wadmark(f):
    N = len(f)
    F = zeros(N)
    alpha = 10.0**(-0.2)
    
    for i in range(N):
        if( f[i] < alpha ):
            F[i] = - 40
        else:
            F[i] = - 50 - 50*log10(f[i])

    return F
    
