#pragma once

#include "BaseFilter.h"

namespace robot {

/**
 * 
 * x_k = x_{k-1} + w_k  (过程模型，w_k ~ N(0,Q))
 * z_k = x_k + v_k      (观测模型，v_k ~ N(0,R))
 */

struct LiteKalmanFilterParm {
  double Q = 0.001;
  double R = 0.5;
  double initial_value = 0.0;
};

class LiteKalmanFilter : public BaseFilter {
public:
  explicit LiteKalmanFilter(const LiteKalmanFilterParm& p);
  double filter(double input) override;
  void reset(double initial_value = 0.0);
  

private:
  double Q_, R_; 
  double Pm_, Pk_, Kk_, out_;
  bool is_initialized_ = false; // 跟踪初始化状态
};

}




