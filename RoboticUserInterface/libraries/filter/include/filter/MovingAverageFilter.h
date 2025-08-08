#pragma once

#include <vector>
#include "BaseFilter.h"

namespace robot {

struct MovingAverageFilterParm {
  int window_size = 10;
};

class MovingAverageFilter : public BaseFilter {
public:
  explicit MovingAverageFilter(const MovingAverageFilterParm& p);
  double filter(double input) override;

private:
  std::vector<double> data_;
  int window_size_;
};

}





