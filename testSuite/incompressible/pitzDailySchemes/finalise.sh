#!/bin/sh
mv log pimpleFoam.log
foamJob -p -s postProcess -func sampleDict -latestTime >& sampleDict.log
