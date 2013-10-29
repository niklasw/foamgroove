#!/bin/bash

for d in *; do
    if [ -d "$d" ]; then
        (
            cd $d
            rm archive.tgz
            tar czf archive.tgz *
        )
    fi
done
