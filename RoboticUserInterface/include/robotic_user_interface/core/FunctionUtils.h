#pragma once

#include <chrono>
#include <cstdint>  
#include "Eigen/Dense"

/**
 * Return current timestamp in milliseconds.
 */
uint64_t timestamp_ms();

/**
 * Return current timestamp in milliseconds.
 */
double timestamp_ms_f();

/**
 * Return current timestamp in nanoseconds.
 */
uint64_t timestamp_ns();

/**
 * Return euler angles XYZ from a quaternion. When a reference yaw is given, the yaw angle is chosen as close as possible to the
 * reference.
 */
template <typename scalar_t>
Eigen::Matrix<scalar_t, 3, 1> eulerXYZFromQuaternion(const Eigen::Quaternion<scalar_t>& quat, scalar_t referenceYaw);

/**
 * Return euler angles ZYX from a quaternion. When a reference yaw is given, the yaw angle is chosen as close as possible to the
 * reference.
 */
template <typename scalar_t>
Eigen::Matrix<scalar_t, 3, 1> eulerZYXFromQuaternion(const Eigen::Quaternion<scalar_t>& quat, scalar_t referenceYaw);

