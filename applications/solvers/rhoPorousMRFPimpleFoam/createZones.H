    MRFZones mrfZones(mesh);
    mrfZones.correctBoundaryVelocity(U);

    thermalPorousZones pZones(mesh);
    Switch pressureImplicitPorosity(false);
