    Info<< "Reading field U (for boundary conditions)\n" << endl;

    volVectorField U
    (
        IOobject
        (
            "U",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

    volVectorField Uexact
    (
        IOobject
        (
            "Uexact",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        U
    );

    const volVectorField& cellCentres = mesh.C();

    forAll(cellCentres, I)
    {
        const vector& c = cellCentres[I];
        vector R = c-cylCentre;
        R -= cylAxis*(R & cylAxis); //cylAxis is normalised
        scalar magR = mag(R);

        scalar sinTheta = mag(R^flowVector)/(mag(R)*mag(flowVector));
        scalar cosTheta = Foam::sqrt(1-sqr(sinTheta));
        //Info << R << "\t" << flowVector <<"\t" << (R^flowVector)
        //     << "\t" <<sinTheta << "\t" << cosTheta << endl;

        scalar cos2Theta = 1-2*sqr(sinTheta);
        scalar sin2Theta = 2*sinTheta*cosTheta;

        Uexact[I].component(vector::X) = 1-sqr(magR/cylDiameter)*cos2Theta;
        Uexact[I].component(vector::Z) =   sqr(magR/cylDiameter)*sin2Theta;
        Uexact[I].component(vector::Y) =   0.0;
    }

    Info << "Writing field Uexact to time = " << runTime.timeName() << endl;

    Uexact.write();

