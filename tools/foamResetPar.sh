#!/usr/bin/env sh
read -p "Really reset? Ctrl-C to cancel."
foamListTimes -rm -withZero -processor
decomposePar -fields -copyZero
