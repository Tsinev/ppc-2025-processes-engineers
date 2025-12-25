#include "timur_a_integral/seq/include/ops_seq.hpp"

#include <cmath>

#include "timur_a_integral/common/include/common.hpp"

namespace timur_a_integral {

TimurAIntegralSEQ::TimurAIntegralSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
}
//
bool TimurAIntegralSEQ::ValidationImpl() {
  return GetInput().n_steps > 0 && GetInput().x2 > GetInput().x1 && GetInput().y2 > GetInput().y1;
}

bool TimurAIntegralSEQ::PreProcessingImpl() {
  GetOutput() = 0.0;
  return true;
}

bool TimurAIntegralSEQ::RunImpl() {
 auto data = GetInput();
  auto f = GetFunction(data.func_id);
  double hx = (data.x2 - data.x1) / data.n_steps;
  double hy = (data.y2 - data.y1) / data.n_steps;

  double sum = 0.0;

  for (int i = 1; i < data.n_steps; i++) {
    double x = data.x1 + (i * hx);
    for (int j = 1; j < data.n_steps; j++) {
      double y = data.y1 + (j * hy);
      sum += f(x, y);
    }
  }

  for (int i = 1; i < data.n_steps; i++) {
    double x = data.x1 + (i * hx);
    sum += 0.5 * f(x, data.y1);
    sum += 0.5 * f(x, data.y2);
  }

  for (int j = 1; j < data.n_steps; j++) {
    double y = data.y1 + (j * hy);
    sum += 0.5 * f(data.x1, y);
    sum += 0.5 * f(data.x2, y);
  }

  sum += 0.25 * f(data.x1, data.y1);
  sum += 0.25 * f(data.x2, data.y1);
  sum += 0.25 * f(data.x1, data.y2);
  sum += 0.25 * f(data.x2, data.y2);

  GetOutput() = sum * hx * hy;

  return true;
}

bool TimurAIntegralSEQ::PostProcessingImpl() {
  return true;
}


}  // namespace timur_a_integral
