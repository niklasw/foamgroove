#include "myODE.H"
#include "mathematicalConstants.H"

namespace Foam
{
    myODE::myODE
    (
        const scalar frequency,
        const scalar damping
    )
    :
        omega_(2*constant::mathematical::pi*frequency),
        damping_(damping),
        Q_(0.0)
    {}

    label myODE::nEqns() const
    {
        return 2;
    }

    void myODE::RHS(scalar Q)
    {
        Q_ = Q;
    }

    void myODE::derivatives
    (
        const scalar x,
        const scalarField& y,
        scalarField& dydx
    ) const
    {
        dydx[0] = y[1];
        dydx[1] = Q_
                - 2*damping_*omega_*y[1]
                - pow(omega_,2)*y[0];
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
        dfdx[1] = omega_*1.1*cos(x*omega_*1.1);//0.0;

        dfdy[0][0] = 0.0;
        dfdy[0][1] = 1.0;
        dfdy[1][0] = -pow(omega_,2);
        dfdy[1][1] = -2*damping_*omega_;
    }
} // END namespace Foam


