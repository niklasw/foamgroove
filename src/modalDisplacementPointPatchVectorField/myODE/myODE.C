#include "myODE.H"

namespace Foam
{
    myODE::myODE
    (
        const scalar omega,
        const scalar Q,
        const scalarField& X
    )
    :
        omega_(omega),
        Q_(Q),
        X_(X)
    {}

    label myODE::nEqns() const
    {
        return 2;
    }


    void myODE::derivatives
    (
        const scalar x,
        const scalarField& y,
        scalarField& dydx
    ) const
    {
        dydx[0] = y[1];
        dydx[1] = Q_-pow(omega_,2)*y[0];
    }

    void myODE::jacobian
    (
        const scalar x,
        const scalarField& y,
        scalarField& dfdx,
        scalarSquareMatrix& dfdy
    ) const
    {
        dfdx[0] = 0.0;
        dfdx[1] = 0.0;

        dfdy[0][0] = 0.0;
        dfdy[0][1] = 1.0;
        dfdy[1][0] = -pow(omega_,2);
        dfdy[1][1] = 0.0;
    }
} // END namespace Foam


