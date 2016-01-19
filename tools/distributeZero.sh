#!/bin/bash

distributeZero()
{
    for p in processor*; do
        rm -rf $p/0
        cp -rv 0 $p/0
    done
}

distributeZero

