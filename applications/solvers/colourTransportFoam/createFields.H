    IObasicSourceList sources(mesh);


    Info<< "Reading or creating field T\n" << endl;

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

    Info<< "Reading field U\n" << endl;

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

    Info<< "Reading transportProperties\n" << endl;

    IOdictionary transportProperties
    (
        IOobject
        (
            "transportProperties",
            runTime.constant(),
            mesh,
            IOobject::MUST_READ_IF_MODIFIED,
            IOobject::NO_WRITE
        )
    );


    Info<< "Reading diffusivity D\n" << endl;

    dimensionedScalar DT
    (
        transportProperties.lookupOrDefault("DT",dimensionedScalar("DT",dimensionSet(0,2,-1,0,0),0.0))
    );

#   include "createPhi.H"
