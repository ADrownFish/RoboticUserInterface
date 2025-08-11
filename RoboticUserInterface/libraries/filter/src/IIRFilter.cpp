#include "filter/IIRFilter.h"
#include "iir/Iir.h"

namespace robot {

IIRFilterParm::LowHighPass::LowHighPass() {
  sampleRate = 0.0;
  cutoffFrequency = 0.0;
  order = 1;
}

IIRFilterParm::LowHighPass::LowHighPass(int _order, double _sampleRate, double _cutoffFrequency) {
  sampleRate = _sampleRate;
  cutoffFrequency = _cutoffFrequency;
  order = _order;
}

// ========== BandStopPass ==========
IIRFilterParm::BandStopPass::BandStopPass() {
  sampleRate = 0.0;
  centerFrequency = 0.0;
  widthFrequency = 0.0;
  order = 1;
}

IIRFilterParm::BandStopPass::BandStopPass(int _order, double _sampleRate, double _centerFrequency, double _widthFrequency) {
  sampleRate = _sampleRate;
  centerFrequency = _centerFrequency;
  widthFrequency = _widthFrequency;
  order = _order;
}

// ========== Shelf ==========
IIRFilterParm::Shelf::Shelf() {
  sampleRate = 0.0;
  cutoffFrequency = 0.0;
  gainDb = 0.0;
  order = 1;
}

IIRFilterParm::Shelf::Shelf(int _order, double _sampleRate, double _cutoffFrequency, double _gainDb) {
  sampleRate = _sampleRate;
  cutoffFrequency = _cutoffFrequency;
  gainDb = _gainDb;
  order = _order;
}

// ========== Value ==========
IIRFilterParm::Value::Value() {
  parm[0] = parm[1] = parm[2] = 0.0;
  order = 1;
}

IIRFilterParm::Value::Value(int _order, double _sampleRate, double _cutoffFrequency, double _gainDb) {
  parm[0] = _sampleRate;
  parm[1] = _cutoffFrequency;
  parm[2] = _gainDb;
  order = _order;
}

// ========== IIRFilterParm ==========
IIRFilterParm::IIRFilterParm() {
  value = Value();
}

IIRFilterParm::IIRFilterParm(Value _value) {
  value = _value;
}

IIRFilterParm::IIRFilterParm(LowHighPass _lowHighPass) {
  lowHighPass = _lowHighPass;
}

IIRFilterParm::IIRFilterParm(BandStopPass _bandStopPass) {
  bandStopPass = _bandStopPass;
}

IIRFilterParm::IIRFilterParm(Shelf _shelf) {
  shelf = _shelf;
}

IIRFilter::IIRFilter() {}
IIRFilter::IIRFilter(IIRFilterType type,
                     const IIRFilterParm &parm) {
  createFilter(type, parm);
}

IIRFilter::~IIRFilter() {}

void IIRFilter::createFilter(IIRFilterType type,
                             const IIRFilterParm &parm) {

  _m_Initialized = true;
  _m_type = type;

  try {

    switch (type) {
    case IIRFilterType::LowPass:
      _m_filter = std::make_shared<Iir::Butterworth::LowPass<>>();
      static_cast<Iir::Butterworth::LowPass<> *>(_m_filter.get())
          ->setup(parm.value.order, parm.value.parm[0], parm.value.parm[1]);

      break;
    case IIRFilterType::HighPass:
      _m_filter = std::make_shared<Iir::Butterworth::HighPass<>>();
      static_cast<Iir::Butterworth::HighPass<> *>(_m_filter.get())
          ->setup(parm.value.order, parm.value.parm[0], parm.value.parm[1]);

      break;
    case IIRFilterType::BandPass:
      _m_filter = std::make_shared<Iir::Butterworth::BandPass<>>();
      static_cast<Iir::Butterworth::BandPass<> *>(_m_filter.get())
          ->setup(parm.value.order, parm.value.parm[0], parm.value.parm[1],
                  parm.value.parm[2]);

      break;
    case IIRFilterType::BandStop:
      _m_filter = std::make_shared<Iir::Butterworth::BandStop<>>();
      static_cast<Iir::Butterworth::BandStop<> *>(_m_filter.get())
          ->setup(parm.value.order, parm.value.parm[0], parm.value.parm[1],
                   parm.value.parm[2]);

      break;
    case IIRFilterType::LowShelf:
      _m_filter = std::make_shared<Iir::Butterworth::LowShelf<>>();
      static_cast<Iir::Butterworth::LowShelf<> *>(_m_filter.get())
          ->setup(parm.value.order, parm.value.parm[0], parm.value.parm[1],
                  parm.value.parm[2]);

      break;
    case IIRFilterType::HighShelf:
      _m_filter = std::make_shared<Iir::Butterworth::HighShelf<>>();
      static_cast<Iir::Butterworth::HighShelf<> *>(_m_filter.get())
          ->setup(parm.value.order, parm.value.parm[0], parm.value.parm[1],
                  parm.value.parm[2]);

      break;

    default:
      _m_Initialized = false;
      _m_type = IIRFilterType::None;
      std::cerr << "\033[31m"
                << "[ IIRFilter ][ Error ] No filter matched \033[0m\n";
      break;
    };
  } catch (const std::exception &e) {
    std::cerr << "\033[31m" << "[ IIRFilter ][ Error ] " << e.what()
              << "\033[0m\n";
  }
}

double IIRFilter::filter(double input) {

  if (_m_Initialized) {
    switch (_m_type) {
    case IIRFilterType::LowPass:
      return static_cast<Iir::Butterworth::LowPass<> *>(_m_filter.get())
          ->filter(input);
      break;
    case IIRFilterType::HighPass:
      return static_cast<Iir::Butterworth::HighPass<> *>(_m_filter.get())
          ->filter(input);
      break;
    case IIRFilterType::BandPass:
      return static_cast<Iir::Butterworth::BandPass<> *>(_m_filter.get())
          ->filter(input);
      break;
    case IIRFilterType::BandStop:
      return static_cast<Iir::Butterworth::BandStop<> *>(_m_filter.get())
          ->filter(input);
      break;
    case IIRFilterType::LowShelf:
      return static_cast<Iir::Butterworth::LowShelf<> *>(_m_filter.get())
          ->filter(input);
      break;
    case IIRFilterType::HighShelf:
      return static_cast<Iir::Butterworth::HighShelf<> *>(_m_filter.get())
          ->filter(input);
      break;

    default:
      return input;
      break;
    }
  } else {
    return input;
  }
}
}