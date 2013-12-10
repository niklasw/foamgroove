#!/bin/bash
# Find and copy foamDictionary to the case 
# you are standing in. Simply searches the 
# foam applications directories for the 
# specified dictionary.
# Usage:
#
# foamFetchDict.sh createPatch system
# 
# This will search for createPatchDict or
# createPatchProperties and copy if found
# to system.


dictName=$1
dictName2=$1
target=$2
dictName=${dictName:=decomposePar}Dict
dictName2=${dictName2:=extrude}Properties

target=${target:=system}

[ -d $target ]  || { echo "No $target dir"; exit 1; }

echo "...trying $dictName";
dictPath=$( find  $FOAM_UTILITIES -name $dictName )
if [ ! -z "$dictPath" ]; then
    echo Found $dictPath
else
    echo "Found no dict named $dictName";
    echo "...trying $dictName2";
    dictName=$dictName2
    dictPath=$( find $FOAM_UTILITIES -name $dictName )
    if [ -z $dictPath ]; then
        echo "Found no dict named $dictName" either. Giving up.;
        exit 1
    fi
fi

echo "${dictName} > $target "
cp $dictPath $target/.
