#!/bin/bash

# Change some switches in $WM_PROJECT_DIR/etc/controlDict

if [ -z $WM_PROJECT_DIR ]; then
    echo -e "OpenFOAM not initialised"
    exit 1
fi

ETC="$WM_PROJECT_DIR/etc/controlDict"

if [ ! -w $WM_PROJECT_DIR ]; then
    echo -e "Cannot write to $WM_PROJECT_DIR"
    exit 2
fi

cat $ETC > ${ETC}.bak

sed    -e 's/\(allowSystemOperations \).*;/\1 1;/' ${ETC}.bak > $ETC
sed -i -e 's/\(fileModificationChecking \).*;/\1 timeStamp;/' $ETC





