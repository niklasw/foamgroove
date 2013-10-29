   {
       Info << "Thermodynamics: " << "T, average/max/min = " << thermo.T().average().value()
            <<" "<< gMax(thermo.T()) << " " << gMin(thermo.T()) << endl;
       Info << "Thermodynamics: " << "p, average/max/min = " << p.average().value()
            <<" "<< gMax(p) << " " << gMin(p) << endl;
   }

