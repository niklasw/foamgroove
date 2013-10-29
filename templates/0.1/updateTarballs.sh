#!/bin/bash 
for f in *.tgz; do
    name=${f%.*}
    [[ -d "$name" ]] || continue;
    echo -e "$name -> $f"
    tar czf $f $name
done
