#!/bin/bash

rm -rf processor*/0
for p in processor*; do
    echo "\n$p"
    cp -r 0 $p/0
done


