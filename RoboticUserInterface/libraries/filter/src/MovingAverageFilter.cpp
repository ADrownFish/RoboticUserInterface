#include "filter/MovingAverageFilter.h"
#include <math.h>

namespace robot {

MovingAverageFilter::MovingAverageFilter(const MovingAverageFilterParm& p) {
    window_size_= p.window_size;
}

double MovingAverageFilter::filter(double input) {
    data_.push_back(input);

    if (data_.size() < window_size_){
        return input;
    }

    if (data_.size() > window_size_){
        data_.erase(data_.begin());
    }

    double sum = 0.0;
    for (double value : data_) {
        sum += value;
    }

    double output = sum / data_.size();
    return output;
}

}



