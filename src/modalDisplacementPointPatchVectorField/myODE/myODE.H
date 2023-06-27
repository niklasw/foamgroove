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
    scalar damping_;
    scalar Q_;

public:
    myODE
    (
        const scalar frequency,
        const scalar damping
    );

    label nEqns() const;

    void RHS(scalar Q);

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
