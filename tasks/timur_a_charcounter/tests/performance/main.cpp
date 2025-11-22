#include <gtest/gtest.h>

#include "timur_a_charcounter/common/include/common.hpp"
#include "timur_a_charcounter/mpi/include/ops_mpi.hpp"
#include "timur_a_charcounter/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace timur_a_charcounter {

class ExampleRunPerfTestProcesses : public ppc::util::BaseRunPerfTests<InType, OutType> {
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

TEST_P(ExampleRunPerfTestProcesses, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, TimurACharcounterMPI, TimurACharcounterSEQ>(PPC_SETTINGS_timur_a_charcounter);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = ExampleRunPerfTestProcesses::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, ExampleRunPerfTestProcesses, kGtestValues, kPerfTestName);

}  // namespace timur_a_charcounter
