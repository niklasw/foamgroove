#!/bin/bash


outData=$1
shift 1
baseData=$1
shift 1
appendData="$@"

cat $baseData > $outData

for f in $appendData; do
    sed -n -e '/FIELD attributes/,$p' $f >> $outData
done

