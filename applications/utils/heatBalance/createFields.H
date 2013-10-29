    Info<< "Reading thermophysical properties\n" << endl;

    autoPtr<basicThermo> pThermo
    (
        basicThermo::New(mesh)
    );
    basicThermo& thermo = pThermo();

    const volScalarField& h = thermo.h();

    IOobject UHdr
    (
        "U",
        runTime.timeName(),
        mesh,
        IOobject::MUST_READ
    );

    IOobject pHdr
    (
        "p",
        runTime.timeName(),
        mesh,
        IOobject::MUST_READ
    );

    IOobject THdr
    (
        "T",
        runTime.timeName(),
        mesh,
        IOobject::MUST_READ
    );

    IOobject rhoHdr
    (
        "rho",
        runTime.timeName(),
        mesh,
        IOobject::MUST_READ
    );

    volVectorField U(UHdr, mesh);
    volScalarField p(pHdr, mesh);
    volScalarField T(THdr, mesh);
    volScalarField rho(rhoHdr, mesh);



