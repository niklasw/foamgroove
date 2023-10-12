#!/bin/sh
postProcess -func 'patchAverage(name=top,wallShearStressMean)'
postProcess -func 'patchAverage(name=bottom,wallShearStressMean)'
postProcess -func sampleDict
pimpleFoam -postProcess -func 'yPlus'
