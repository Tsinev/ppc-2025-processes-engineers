#pragma once

#include "timur_a_image/common/include/common.hpp"
#include "task/include/task.hpp"

namespace timur_a_image {

class TimurAImageSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit TimurAImageSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace timur_a_image
