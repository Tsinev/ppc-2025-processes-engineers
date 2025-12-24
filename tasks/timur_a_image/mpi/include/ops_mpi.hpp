#pragma once

#include "timur_a_image/common/include/common.hpp"
#include "task/include/task.hpp"

namespace timur_a_image {

class TimurAImageMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit TimurAImageMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace timur_a_image
