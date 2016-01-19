BEGIN {
    sumP = 0;
    timeCounter=0;
    newTime=0;
    totalPiters = 0;
    oldClockTime = 0;
    deltaTime = 0;
    maxCourant = 1;
}

{
    if ( $1 == "Time" && $2 == "=" ) {
        curTime = $3
        timeCounter++;
        newTime = 1;
        totalUiters+=curUIters
        totalPiters+=curPIters;
        printf("%15f%10i%10i%10is%12.3f\n",curTime,curPIters,curUIters,deltaTime,maxCourant)
        curPIters=0;
        curUIters=0;
    }


    if ( $1 == "ExecutionTime" ) {
        clockTime = $7;
        deltaTime = clockTime - oldClockTime;
        oldClockTime = clockTime
    }

    if ( $2 == "Solving" && $4 ~ "U" ){
        curUIters += $15;
    }

    if ( $2 == "Solving" && $4 == "p," ){
        curPIters += $15;
    }

    if ( $1 == "Courant" && $2 == "Number"){
        maxCourant = $6;
    }
}

END {
    printf("%15s%10s%10s%10s%12s", "time","p-iters", "U-iters", "clock", "CFL \n")
    printf("\nNumber of time steps in file   = %i\n", timeCounter)
    printf("\nTotal number of p iterations   = %i", totalPiters)
    printf("\nTotal number of U iterations   = %i", totalUiters)
    printf("\nTotal number of p+U iterations = %i\n", totalUiters+totalPiters)
}


