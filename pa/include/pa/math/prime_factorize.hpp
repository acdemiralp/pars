#ifndef PA_MATH_PRIME_FACTORIZE_HPP
#define PA_MATH_PRIME_FACTORIZE_HPP

#include <cmath>
#include <vector>

#include <pa/export.hpp>

namespace pa
{
template<typename type>
PA_EXPORT std::vector<type> prime_factorize(type value)
{
  std::vector<type> prime_factors;
  
  type denominator(2);
  while (std::pow(denominator, type(2)) <= value)
  {
    if (value % denominator == type(0))
    {
      prime_factors.push_back(denominator);
      value /= denominator;
    }
    else
      ++denominator;
  }

  if (value > type(1))
    prime_factors.push_back(value);

  return prime_factors;
}
}

#endif