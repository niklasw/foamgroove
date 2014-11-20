#!/bin/bash

function runApp()
{
    [[ "$1" == "-p" ]] && { pflag=$1; shift 1; }
    app=$1
    shift 1
    foamJob $pflag $app "$@"
    mv log $app.log
}

runApp renumberMesh -overwrite
decomposePar -force
runApp -p setFields
runApp -p interFoam
