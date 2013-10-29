{

    tmp<surfaceScalarField> uPhi = linearInterpolate(U) & mesh.Sf();
    tmp<fvScalarMatrix> colourEqn
    (
        fvm::ddt(colour)+fvm::div(uPhi, colour) == sources(colour)
    );

    sources.constrain(colourEqn());

    colourEqn().solve();

    /*
    colour.max(0);
    colour.min(1);
    */
}
