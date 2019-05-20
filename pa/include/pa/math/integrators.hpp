#ifndef PA_MATH_INTEGRATORS_HPP
#define PA_MATH_INTEGRATORS_HPP

#include <variant>

#include <boost/numeric/odeint.hpp>

#include <pa/math/types.hpp>

namespace pa
{
using value_type                              = scalar ;
using state_type                              = vector4;
using algebra_type                            = boost::numeric::odeint::vector_space_algebra;

using euler_integrator                        = boost::numeric::odeint::euler                  <   state_type, value_type, state_type, value_type, algebra_type>;
using modified_midpoint_integrator            = boost::numeric::odeint::modified_midpoint      <   state_type, value_type, state_type, value_type, algebra_type>;
using runge_kutta_4_integrator                = boost::numeric::odeint::runge_kutta4           <   state_type, value_type, state_type, value_type, algebra_type>;
using runge_kutta_cash_karp_54_integrator     = boost::numeric::odeint::runge_kutta_cash_karp54<   state_type, value_type, state_type, value_type, algebra_type>;
using runge_kutta_dormand_prince_5_integrator = boost::numeric::odeint::runge_kutta_dopri5     <   state_type, value_type, state_type, value_type, algebra_type>;
using runge_kutta_fehlberg_78_integrator      = boost::numeric::odeint::runge_kutta_fehlberg78 <   state_type, value_type, state_type, value_type, algebra_type>;
using adams_bashforth_2_integrator            = boost::numeric::odeint::adams_bashforth        <2, state_type, value_type, state_type, value_type, algebra_type>;
using adams_bashforth_moulton_2_integrator    = boost::numeric::odeint::adams_bashforth_moulton<2, state_type, value_type, state_type, value_type, algebra_type>;

using variant_integrator                      = std::variant<
  euler_integrator                            , 
  modified_midpoint_integrator                ,
  runge_kutta_4_integrator                    , 
  runge_kutta_cash_karp_54_integrator         , 
  runge_kutta_dormand_prince_5_integrator     , 
  runge_kutta_fehlberg_78_integrator          , 
  adams_bashforth_2_integrator                ,
  adams_bashforth_moulton_2_integrator        >;
}

#endif