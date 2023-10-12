#!/bin/bash

for c in subCase_*
do
    simtime=$(grep ExecutionTime $c/run.stdout|tail -n1)
    nsteps=$(grep -c ExecutionTime $c/run.stdout)
    echo -e "${c}:\n\tsimulationTime = $simtime\n\tnumber of timesteps = $nsteps\n"
done
