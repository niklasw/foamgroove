#!/bin/bash

method=$1
np=$2

function sanityCheck()
{
    S=0
    [ -f "$FOAM_UTILITIES/parallelProcessing/decomposePar/decomposeParDict" ] || S=1;
    [ -d "system" ] || S=${S}2
    echo $S
}

if [ $(sanityCheck) == "0" ]; then
    [ -f system/decomposeParDict ] || cp "$FOAM_UTILITIES/parallelProcessing/decomposePar/decomposeParDict" "system/.";
    sed -i -e "s%\(method *\).*;%\1$method;%" system/decomposeParDict
    if [ -n "$np" ]; then
        sed -i -e "s%\(numberOfSubdomains *\).*;%\1$np;%" system/decomposeParDict
    fi
else
    echo "Error. Nothing done"
fi

if [ "$method" != "scotch" ]; then
    vim system/decomposeParDict +/${method}Coeffs
fi
