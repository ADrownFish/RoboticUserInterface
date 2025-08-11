#pragma once

#include "BaseFilter.h"

namespace Iir {
  class PoleFilterBase2;
}

namespace robot {
enum class IIRFilterType {
  None,
  LowPass,
  HighPass,
  BandPass,
  BandStop,
  LowShelf,
  HighShelf,
};

union IIRFilterParm {
  struct LowHighPass {
    LowHighPass();
    LowHighPass(int _order, double _sampleRate, double _cutoffFrequency);
    double sampleRate;
    double cutoffFrequency;
    int order;
  };

  struct BandStopPass {
    BandStopPass();
    BandStopPass(int _order, double _sampleRate, double _centerFrequency, double _widthFrequency);
    double sampleRate;
    double centerFrequency;
    double widthFrequency;
    int order;
  };

  struct Shelf {
    Shelf();
    Shelf(int _order, double _sampleRate, double _cutoffFrequency, double _gainDb);
    double sampleRate;
    double cutoffFrequency;
    double gainDb;
    int order;
  };

  struct Value {
    Value();
    Value(int _order, double _sampleRate, double _cutoffFrequency, double _gainDb);
    double parm[3];
    int order;
  };

  IIRFilterParm();
  IIRFilterParm(Value _value);
  IIRFilterParm(LowHighPass _lowHighPass);
  IIRFilterParm(BandStopPass _bandStopPass);
  IIRFilterParm(Shelf _shelf);

  Value value;
  LowHighPass lowHighPass;
  BandStopPass bandStopPass;
  Shelf shelf;
};

class IIRFilter : public BaseFilter {

public:
  IIRFilter();
  
  IIRFilter(IIRFilterType type, const IIRFilterParm &parm);

  ~IIRFilter();

  void createFilter(IIRFilterType type, const IIRFilterParm &parm);

  double filter(double input);

private:
  bool _m_Initialized = false;
  std::shared_ptr<Iir::PoleFilterBase2> _m_filter;
  IIRFilterType _m_type = IIRFilterType::None;
};

} // namespace sevnce