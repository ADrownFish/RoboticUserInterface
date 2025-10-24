#include "robotic_user_interface/core/FunctionUtils.h"
#include "robotic_user_interface/core/Types.h"


namespace {  // helper functions to select the right modulo
template <typename SCALAR_T>
SCALAR_T scalarMod(SCALAR_T, SCALAR_T);

template <>
float scalarMod<float>(float x, float y) {
  return fmodf(x, y);
}

template <>
double scalarMod<double>(double x, double y) {
  return fmod(x, y);
}
}  // namespace

/**
 * @brief Transform a set of Euler Angles (each in [-pi, pi)) into Euler Angles in the range [-pi,pi),[-pi/2,pi/2),[-pi,pi)
 * @param[in,out] Reference to eulerAngles XYZ or ZYX which will be modified in place
 * @note Code taken from https://github.com/ANYbotics/kindr/blob/master/include/kindr/rotations/EulerAnglesXyz.hpp
 * @note Works for Euler Angles XYZ and ZYX alike
 */
template <typename SCALAR_T>
void makeEulerAnglesUnique(Eigen::Matrix<SCALAR_T, 3, 1>& eulerAngles) {
  SCALAR_T tol(1e-9);  // FIXME(jcarius) magic number
  SCALAR_T pi(M_PI);

  if (eulerAngles.y() < -pi / 2 - tol) {
    if (eulerAngles.x() < 0) {
      eulerAngles.x() = eulerAngles.x() + pi;
    } else {
      eulerAngles.x() = eulerAngles.x() - pi;
    }

    eulerAngles.y() = -(eulerAngles.y() + pi);

    if (eulerAngles.z() < 0) {
      eulerAngles.z() = eulerAngles.z() + pi;
    } else {
      eulerAngles.z() = eulerAngles.z() - pi;
    }
  } else if (-pi / 2 - tol <= eulerAngles.y() && eulerAngles.y() <= -pi / 2 + tol) {
    eulerAngles.x() -= eulerAngles.z();
    eulerAngles.z() = 0;
  } else if (-pi / 2 + tol < eulerAngles.y() && eulerAngles.y() < pi / 2 - tol) {
    // ok
  } else if (pi / 2 - tol <= eulerAngles.y() && eulerAngles.y() <= pi / 2 + tol) {
    // todo: pi/2 should not be in range, other formula?
    eulerAngles.x() += eulerAngles.z();
    eulerAngles.z() = 0;
  } else  // pi/2 + tol < eulerAngles.y()
  {
    if (eulerAngles.x() < 0) {
      eulerAngles.x() = eulerAngles.x() + pi;
    } else {
      eulerAngles.x() = eulerAngles.x() - pi;
    }

    eulerAngles.y() = -(eulerAngles.y() - pi);

    if (eulerAngles.z() < 0) {
      eulerAngles.z() = eulerAngles.z() + pi;
    } else {
      eulerAngles.z() = eulerAngles.z() - pi;
    }
  }
}

scalar_t moduloAngleWithReference(scalar_t x, scalar_t reference) {
  const scalar_t ub = reference + M_PI;  // upper bound
  const scalar_t lb = reference - M_PI;  // lower bound

  if (x > ub) {
    x = lb + scalarMod(x - lb, 2.0 * M_PI);
  } else if (x < lb) {
    x = ub - scalarMod(ub - x, 2.0 * M_PI);
  }

  return x;
}

uint64_t timestamp_ms() {
	using namespace std::chrono;
	return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

double timestamp_ms_f() {
	using namespace std::chrono;
	return static_cast<uint64_t>(duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count()) / 1000'000'000.;
}

uint64_t timestamp_ns() {
	using namespace std::chrono;
	return static_cast<uint64_t>(duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count());
}

template <typename scalar_t>
Eigen::Matrix<scalar_t, 3, 1> eulerXYZFromQuaternion(const Eigen::Quaternion<scalar_t>& quat, scalar_t referenceYaw) {
  Eigen::Matrix<scalar_t, 3, 1> eulerXYZ = quat.toRotationMatrix().eulerAngles(0, 1, 2);
  makeEulerAnglesUnique(eulerXYZ);
  eulerXYZ.z() = moduloAngleWithReference(eulerXYZ.z(), referenceYaw);
  return eulerXYZ;
}

template <typename scalar_t>
Eigen::Matrix<scalar_t, 3, 1> eulerZYXFromQuaternion(const Eigen::Quaternion<scalar_t>& quat, scalar_t referenceYaw) {
  Eigen::Matrix<scalar_t, 3, 1> eulerZYX = quat.toRotationMatrix().eulerAngles(2, 1, 0);
  makeEulerAnglesUnique(eulerZYX);
  eulerZYX.x() = moduloAngleWithReference(eulerZYX.x(), referenceYaw);
  return eulerZYX;
}

template Eigen::Matrix<float, 3, 1> eulerXYZFromQuaternion<float>(const Eigen::Quaternion<float>&, float);
template Eigen::Matrix<double, 3, 1> eulerXYZFromQuaternion<double>(const Eigen::Quaternion<double>&, double);

template Eigen::Matrix<float, 3, 1> eulerZYXFromQuaternion<float>(const Eigen::Quaternion<float>&, float);
template Eigen::Matrix<double, 3, 1> eulerZYXFromQuaternion<double>(const Eigen::Quaternion<double>&, double);
