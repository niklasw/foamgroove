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
        const scalar t,
        const scalarField& y,
        scalarField& dydt
    ) const
    {
        dydt[0] = y[1];
        dydt[1] = Q_
                - 2*damping_*omega_*y[1]
                - pow(omega_,2)*y[0];
    }

    void myODE::jacobian
    (
        const scalar t,
        const scalarField& y,
        scalarField& dfdt,
        scalarSquareMatrix& dfdy
    ) const
    {
        dfdt[0] = 0.0;
        dfdt[1] = 0.0;

        dfdy[0][0] = 0.0;
        dfdy[0][1] = 1.0;
        dfdy[1][0] = -pow(omega_,2);
        dfdy[1][1] = -2*damping_*omega_;
    }
} // END namespace Foam


