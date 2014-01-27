#!/bin/bash

rm -rf processor*/0
for p in processor*; do
    cp -rv 0 $p/0
done


