#!/usr/bin/python
from pylab import *
from numpy import *


#--------------------------------------------------------------------
# This file contains all the 'spectral functionality'.
# List of functions:
# - sd_fcn            (Checked in 'testPrograms/oneFreq/prog1.py')
# - spsl_fcn      
# - spl_fcn
# - sd_to_spl
# - checkBIargs
# - intervalOverlap
# - bandIntegrate
# - sigma2T           (Checked in 'testPrograms/oneFreq/prog1.py')
# - sigma2F           (Checked in 'testPrograms/oneFreq/prog1.py')
# - sampleParam       (Checked in 'testPrograms/samplingTest/prog.py')
# - sampleTF          (Checked in 'testPrograms/samplingTest/prog.py')
# - genBands          (Checked in 'testPrograms/octaveBandGen/prog.py')
# - thirdOctBandLevel (Checked in 'testPrograms/cylAcoustics/prog3.py')
#--------------------------------------------------------------------

#--------------------------------------------------------------------
# Computation of the "spectral density" of a pressure signal 
# sampled during a time interval of length T.  
# It is assumed that the sampling is uniform. 
# 
# Arguments: 
#  p - The vector of (N) pressure values.
#  T - The length of the sampling interval.
#
# Output:
#  sd - A vector of (M) spectral density values.
#  f   - The corresponding vector of (M) frequencies.
#
# Note 1: The second half of the FFT-coefficients are redundant
# due to aliasing. N (the number of p-values) should be odd, but
# the program seems to work fine anyway.
#
# Note 2: The physical unit of the spectral density is Pa^2/Hz.
#
# Note 3: The likstromskomponent is not removed from the signal.
#
# Mattias Liefvendahl 17 Nov 2016.
#
def sd_fcn(p,T):
    N = len(p)
    (t,f,dt,df) = sampleTF(0,T,N,False)
    M = len(f)

    ph = np.fft.fft(p)
    psd = (2*T/N**2)*np.absolute(ph)**2
    psd = psd[0:M]
    
    return (f,psd)

#------------------------------------------------------
# If dist = 1.0:
# Calculate the sound pressure level SPL of the signal.
#  [dB re 1muPa]
#
# If dist is given:
# Assume that the signal is registered at this distance and
# calculate the corresponding source level.
#  [dB re 1muPa, 1m]
#
# The scalar value is returned.
#
def spl_fcn(p,pRef,dist=1.0):
    spl = 20*log10(std(p)/pRef) + 20*log10(dist)
    return spl


#------------------------------------------------------
# Sound-pressure spectrum level
#
def spsl_fcn(p,T,pRef,dist=1.0,dfRef=1.0):
    (f,psd) = sd_fcn(p,T)
    spl = sd_to_spl(psd,pRef,dfRef) + 20*log10(dist)
    return (f,spl)


#--------------------------------------------------
# Convert from spectral density to SPL
def sd_to_spl(psd,pRef,dfRef=1.0):
    alpha = pRef**2/dfRef
    spl = 10*log10(psd/alpha)
    return spl
    
#-----------------------------------------------
# Check frequency arguments to 'bandIntegrate'
def checkBIargs(f,f1,f2):
    
    # Check limits
    if (f1 > f2):
        print "Error: f1 > f2"

    # Check lower and upper limits with array
    if (f1 > f[-1]):
        print "Error: Band above data range."
        print " f1 = ",f1
        print " f[-1] = ",f[-1]
    if (f2 < f[0]) :
        print "Error: Band below data range."

    # Check uniformity of frequencies
    N = len(f)
    df = zeros(N-1)
    for i in range(N-1):
        df[i] = f[i+1] - f[i]
    if( std(df) > 1e-4*mean(df) ):
        print "Warning: Not so uniform frequency sampling"
        print "mean(df) = ",mean(df)
        print "std(df)  = ",std(df)

#--------------------------------
# Function used in 'bandIntegrate'
# Returns the length of the overlap of the intervals (x1,x2) and (y1,y2)
def intervalOverlap(x1,x2,y1,y2):
    if( x2<x1 ): print "Error in 'intervalOverlap': x2<x1"
    if( y2<y1 ): print "Error in 'intervalOverlap': y2<y1"

    dx = 0.0
    
    # Three cases depending on the relation between x2 and (y1,y2)
    if( x2<y1 ):
        dx = 0.0
    elif( x2<y2 ):
        # And now two sub-cases
        if (x1<y1):
            dx = x2 - y1
        else:
            dx = x2 - x1
    else:
        # And now three sub-cases
        if (x1<y1):
            dx = y2 - y1
        elif ( x1<y2 ):
            dx = y2-x1
        else:
            dx = 0.0

    return dx
        
#-----------------------------------------------
# Integrate 'psd' (with corresponding 'f') over the frequency band [f1,f2].
# Second order integration, and works for non-uniform frequency spacing.
# Uses the functions:
#  - checkBIargs
#  - intervalOverlap
#
def bandIntegrate(f,psd,f1,f2):

    checkBIargs(f,f1,f2)

    # Set up intervals
    N = len(f)
    fl = zeros(N)
    fu = zeros(N)
    # Set the first
    df0 = f[1] - f[0]
    fl[0] = f[0] - df0/2
    fu[0] = f[0] + df0/2
    # Set the internal ones
    for i in range(N-2):
        fl[i+1] = (f[i+1] + f[i])/2.0
        fu[i+1] = (f[i+2] + f[i+1])/2.0
    # Set the last one
    df0 = f[N-1] - f[N-2]
    fl[N-1] = f[N-1] - df0/2
    fu[N-1] = f[N-1] + df0/2

    # Loop over all intervals and integrate the overlap with [f1,f2)
    # Five cases (assuming f2>f1)
    I = 0.0
    for i in range(N):
        df = intervalOverlap(f1,f2,fl[i],fu[i])
        I = I + psd[i]*df
    
    return I

#---------------------------------------------------------------
# Power of the signal
# Unit [Pa**2]
def sigma2T(p):
    p = p - mean(p)
    N = len(p)
    sigP = sum(p**2)/N
    return sigP

#---------------------------------------------------------------
# "Power" of the signal from the psd.
# Exclude psd[0] since it should be zero.
# Unit [Pa**2]
def sigma2F(psd,df):
    M = len(psd)
    return sum(psd[1:M])*df 

#---------------------------------------------------------------------
# From a time series (assumed uniform), set up the sampling parameters.
def sampleParam(t):
    N = len(t)
    dt = (t[-1]-t[0])/(N-1)
    T  = N*dt
    df = 1.0/T
    M  = (N-1)/2
    fMax = (M-1)*df
    
    return (N,dt,T,df,fMax)
    
#--------------------------------------------------------------
# Set up time series based on start point, periodicity and number
# samples. If "debug=true" write the output for sampling to screen.
def sampleTF(t0,T,N,debug=False):
    t = zeros(N)
    M = (N-1)/2
    f = zeros(M)

    # Time values
    dt = T/N
    for i in range(N):
        t[i] = t0 + i*dt
        
    # Frequency values
    df = 1.0/T
    for i in range(M):
        f[i] = i*df
        
    if(debug):
        print "-----------------------------------"
        print "Debug info from 'sampleT':"
        print " t0 = ",t0
        print " T  = ",T
        print " N  = ",N
        print " len(t) = ",len(t)
        print " len(f) = ",len(f)
        print "--"
        print " dt = ",dt
        print " t[first] = t[0]   =",t[0]
        print " t[last]  = t[N-1] =",t[N-1]
        print "--"
        print " df = ",df
        print " fmax = f[M-1] = ",f[M-1]
        print "-----------------------------------"

    return (t,f,dt,df)

#----------------------------------------------------
# Generate 1/3 octave band frequencies
#
# fc  = Center frequency
# fl  = Band lower limit
# fu  = Band upper limit
# N   = Number of bands to generate
# ind = Where to start, relative to the band with fc=10Hz.
#       The choice ind=0 gives fc=10Hz as the first band.
#
def genBands(N,ind):
    fc = zeros(N)
    fl = zeros(N)
    fu = zeros(N)
    alpha = 2.0**(1.0/3)

    fc[0] = 10*(alpha**ind)
    fl[0] = fc[0]/sqrt(alpha)
    fu[0] = fc[0]*sqrt(alpha)

    for i in range(N-1):
        fc[i+1] = alpha*fc[i]
        fl[i+1] = alpha*fl[i]
        fu[i+1] = alpha*fu[i]

    return (fc,fl,fu)



#-----------------------------------------------
# Calculate '1/3-octave band source level'
#  ind   =  Index of first band (ind = 0 gives fc[0] = 10 Hz)
#  Nband = Number of bands
#
def thirdOctBandLevel(p,T,ind,Nband,pRef,dist=1.0):

    (f,psd) = sd_fcn(p,T)
    (fc,fl,fu) = genBands(Nband,ind)

    ssl = zeros(len(fc))
    for i in range(len(fc)):
        tmp = bandIntegrate(f,psd,fl[i],fu[i])/(pRef**2)
        ssl[i] = 10*log10(tmp) + 20*log10(dist)

    return (fc,fl,fu,ssl)




















