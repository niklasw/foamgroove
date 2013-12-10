BEGIN{
    dt=1
    print "#Step ExecutionTime ClockTime TimePerStep ExecTimePerStep N_iters"
    oldCT=0 # Store last clock time
    oldET=0 # Store last execution time
}
{
    if ($1 == "Time" && $2 == "=") {
        dt+=1
        nopiters=0
    }
    if ($2 ~ "Solving" && $4 ~"p"){
        nopiters+=$15
    }
    if ($1 ~ "ExecutionTime"){
        CT=$7
        ET=$3
        Tstep=CT-oldCT
        Estep=ET-oldET
        printf("%i\t%f\t%f\t%f\t%f\t%i\n",dt,ET,CT,Tstep,Estep,nopiters)
        oldCT=CT
        oldET=ET
    }
}
