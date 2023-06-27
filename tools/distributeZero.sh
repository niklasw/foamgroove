#!/bin/bash

distributeZero()
{
    __SRC=0
    [[ -z "$1" ]] || __SRC=$1
    for p in processor*; do
        rm -rf $p/0
        cp -rv $__SRC  $p/0
    done
}

distributeZero $1

