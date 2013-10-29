#ifndef myODE_H
#define myODE_H

#include "ODE.H"
#include "ODESolver.H"
#include "RK.H"


namespace Foam
{
class myODE
:
public ODE
{

    scalar omega_;
    scalar Q_;
    const scalarField& X_;

    myODE
    (
        const scalar omega,
        const scalar Q,
        const scalarField& X
    );

    label nEqns() const;

    void derivatives
    (
        const scalar x,
        const scalarField& y,
        scalarField& dydx
    ) const;

    void jacobian
    (
        const scalar x,
        const scalarField& y,
        scalarField& dfdx,
        scalarSquareMatrix& dfdy
     ) const;
};

} // END namespace Foam

#endif
