#include <gtest/gtest.h>

#include "timur_a_integral/common/include/common.hpp"
#include "timur_a_integral/mpi/include/ops_mpi.hpp"
#include "timur_a_integral/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace timur_a_integral {

class TimurAIntegralPerfTest : public ppc::util::BaseRunPerfTests<InType, OutType> {
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

TEST_P(TimurAIntegralPerfTest, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, TimurAIntegralMPI, TimurAIntegralSEQ>(PPC_SETTINGS_timur_a_integral);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = TimurAIntegralPerfTest::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, TimurAIntegralPerfTest, kGtestValues, kPerfTestName);

}  // namespace timur_a_integral
