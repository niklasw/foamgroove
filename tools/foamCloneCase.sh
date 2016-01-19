#!/bin/bash

target="$@"

EXCLUDES=$(mktemp /tmp/clone_XXXX)

cat << EOF >> $EXCLUDES
- **/processor*
- **/*.png
- **/*log*
- **/postProcessing
EOF

rsync -rvP --exclude-from=$EXCLUDES $PWD/ $target

rm $EXCLUDES


