#pragma once

#include "task/include/task.hpp"
#include "timur_a_integral/common/include/common.hpp"

namespace timur_a_integral {

class TimurAIntegralSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit GaseninLMultIntMstepTrapezSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace timur_a_integral
