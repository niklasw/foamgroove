    // Special solution control reader to take care of
    // several kinds of solvers.

    dictionary* solDictPtr = NULL;

    int nNonOrthCorr = 0;

    if ( mesh.solutionDict().found("SIMPLE") )
    {
        solDictPtr = new dictionary(mesh.solutionDict().subDict("SIMPLE"));
    }
    else if  ( mesh.solutionDict().found("PISO") )
    {
        solDictPtr = new dictionary(mesh.solutionDict().subDict("PISO"));
    }
    else if  ( mesh.solutionDict().found("PIMPLE") )
    {
        solDictPtr = new dictionary(mesh.solutionDict().subDict("PIMPLE"));
    }

    if ( solDictPtr != NULL )
    {
        const dictionary& solDict = *solDictPtr;
        nNonOrthCorr = solDict.lookupOrDefault("nNonOrthogonalCorrectors",0);
    }
