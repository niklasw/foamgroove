
    IObasicSourceList sources(mesh);

    // Create the colour field from scratch.
    volScalarField colour
    (
        IOobject
        (
            "colour",
            runTime.timeName(),
            mesh,
            IOobject::READ_IF_PRESENT,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensionedScalar("colour",dimensionSet(0,0,0,0,0), 0.0),
        zeroGradientFvPatchField<scalar>::typeName
    );


