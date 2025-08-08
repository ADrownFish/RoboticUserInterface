#include "filter/LiteKalmanFilter.h"

namespace robot {

LiteKalmanFilter::LiteKalmanFilter(const LiteKalmanFilterParm& p) 
  : Q_(std::abs(p.Q)), R_(std::abs(p.R)) { // 噪声参数非负
  reset(p.initial_value);
}

// 滤波算法增强
double LiteKalmanFilter::filter(double input) {
  if (!is_initialized_) reset(input); // 自动初始化

  // 预测步骤
  Pk_ = Pm_ + Q_;

  // 数值安全处理
  const double denominator = Pk_ + R_;
  Kk_ = (denominator > 1e-6) ? Pk_ / denominator : 0.0;

  // 更新步骤
  out_ += Kk_ * (input - out_);
  Pm_ = (1.0 - Kk_) * Pk_;

  return out_;
}

void LiteKalmanFilter::reset(double initial_value) {
  Pm_ = Pk_ = Kk_ = 0.0;
  out_ = initial_value;
  is_initialized_ = true;
}

}
