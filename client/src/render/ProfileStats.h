#pragma once
#include <algorithm>
#include <cmath>
#include <vector>
namespace RenderProfile {
struct Distribution {
    double average=0,p95=0,maximum=0;
    std::size_t count=0;
};
inline Distribution Summarize(std::vector<double> values) {
    Distribution result;result.count=values.size();
    if(values.empty())return result;
    for(double value:values)result.average+=value;
    result.average/=values.size();
    std::sort(values.begin(),values.end());
    // Percentil por rango más próximo, sin interpolar inventando muestras.
    result.p95=values[static_cast<std::size_t>(std::ceil(values.size()*.95))-1];
    result.maximum=values.back();return result;
}
}
