{
    fvScalarMatrix hEqn
    (
        fvm::ddt(rho, h)
      + fvm::div(phi, h)
      - fvm::laplacian(turbulence->alphaEff(), h)
     ==
        dpdt
      - (fvc::ddt(rho, K) + fvc::div(phi, K))
    );

    pZones.addEnthalpySource(thermo, rho, hEqn);

    hEqn.relax();
    hEqn.solve();

    thermo.correct();
}
