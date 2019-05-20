#ifndef PA_MATH_TYPES_HPP
#define PA_MATH_TYPES_HPP

#define EIGEN_USE_MKL 
#define EIGEN_USE_MKL_ALL
#define MKL_DIRECT_CALL 

#include <cstdint>

#include <Eigen/Core>

namespace pa
{
using scalar   = float          ;
using vector2  = Eigen::Vector2f;
using vector3  = Eigen::Vector3f;
using vector4  = Eigen::Vector4f;

using integer  = std::int32_t   ;
using ivector2 = Eigen::Vector2i;
using ivector3 = Eigen::Vector3i;
using ivector4 = Eigen::Vector4i;
}

#endif