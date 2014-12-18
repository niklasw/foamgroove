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

#   include "createPhi.H"

    singlePhaseTransportModel laminarTransport(U, phi);

    autoPtr<incompressible::LESModel> LESModel
    (
        incompressible::LESModel::New(U, phi, laminarTransport)
    );

    // Read template k field, to later fill with
    // LESmodel->k().internalField
    autoPtr<volScalarField> kPtr;

    if (IOobject("k", runTime.timeName(), mesh).headerOk())
    {
        kPtr.set
        (
            new volScalarField
            (
                IOobject
                (
                    "k",
                    runTime.timeName(),
                    mesh,
                    IOobject::MUST_READ,
                    IOobject::AUTO_WRITE
                ),
                mesh
            )
        );
    }
    else
    {
        FatalErrorIn("createFields.H") << "Empty k field needed"
            << abort(FatalError);
    }

    volScalarField& k = kPtr();

