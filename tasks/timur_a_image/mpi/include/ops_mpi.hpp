#pragma once

#include "task/include/task.hpp"
#include "timur_a_image/common/include/common.hpp"

namespace timur_a_image {

// MPI implementation of the image processing task
class TimurAImageMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }

  // Constructor that initializes the task with input data
  explicit TimurAImageMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace timur_a_image
