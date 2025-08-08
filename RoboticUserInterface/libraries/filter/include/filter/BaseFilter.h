#pragma once

#include <memory>
#include <array>

namespace robot {

class BaseFilter {
public:
  using Ptr = std::shared_ptr<BaseFilter>;

  BaseFilter() {}
  virtual ~BaseFilter() {}

  virtual double filter(double value) {return value;}
};

}