#include <gtest/gtest.h>

#include "timur_a_symboli/common/include/common.hpp"
#include "timur_a_symboli/mpi/include/ops_mpi.hpp"
#include "timur_a_symboli/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace timur_a_symboli {

class TimurASymboliPerfTest : public ppc::util::BaseRunPerfTests<InType, OutType> {
  const int kCount_ = 100;
  InType input_data_{};

  void SetUp() override {
    input_data_ = kCount_;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return input_data_ == output_data;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(TimurASymboliPerfTest, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, TimurASymboliMPI, TimurASymboliSEQ>(PPC_SETTINGS_timur_a_symboli);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = TimurASymboliPerfTest::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, TimurASymboliPerfTest, kGtestValues, kPerfTestName);

}  // namespace timur_a_symboli
