#include "timur_a_integral/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cmath>
#include <stdexcept>

#include "timur_a_integral/common/include/common.hpp"

namespace timur_a_integral {

TimurAIntegralMPI::TimurAIntegralMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
}

bool TimurAIntegralMPI::ValidationImpl() {
  return GetInput().n_steps > 0 && GetInput().x2 > GetInput().x1 && GetInput().y2 > GetInput().y1;
}

bool TimurAIntegralMPI::PreProcessingImpl() {
  GetOutput() = 0.0;
  return true;
}

namespace {

template <typename Func>
double ComputeIntegralSEQStyle(const TaskData &data, int rank, int size, const Func &f) {
  double hx = (data.x2 - data.x1) / data.n_steps;
  double hy = (data.y2 - data.y1) / data.n_steps;

  int total_inner_i = data.n_steps - 1;

  int points_per_proc = total_inner_i / size;
  int remainder = total_inner_i % size;

  int start_i = rank * points_per_proc + std::min(rank, remainder);
  int end_i = start_i + points_per_proc + (rank < remainder ? 1 : 0);

  start_i += 1;
  end_i += 1;

  double local_sum = 0.0;

  for (int i = start_i; i < end_i; ++i) {
    double x = data.x1 + i * hx;
    for (int j = 1; j < data.n_steps; ++j) {
      double y = data.y1 + j * hy;
      local_sum += f(x, y);
    }
  }

  for (int i = start_i; i < end_i; ++i) {
    double x = data.x1 + i * hx;
    local_sum += 0.5 * f(x, data.y1);
    local_sum += 0.5 * f(x, data.y2);
  }

  return local_sum;
}
}  // namespace

bool TimurAIntegralMPI::RunImpl() {
  try {
    int rank = 0, size = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    TaskData data{};
    if (rank == 0) {
      data = GetInput();
    }

    MPI_Bcast(&data.n_steps, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&data.func_id, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&data.x1, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&data.x2, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&data.y1, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&data.y2, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    auto f = GetFunction(data.func_id);

    double local_sum = 0.0;

    local_sum = ComputeIntegralSEQStyle(data, rank, size, f);

    double total_sum = 0.0;
    MPI_Reduce(&local_sum, &total_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    double result = 0.0;
    if (rank == 0) {
      double hx = (data.x2 - data.x1) / data.n_steps;
      double hy = (data.y2 - data.y1) / data.n_steps;

      for (int j = 1; j < data.n_steps; ++j) {
        double y = data.y1 + j * hy;
        total_sum += 0.5 * f(data.x1, y);
        total_sum += 0.5 * f(data.x2, y);
      }

      total_sum += 0.25 * f(data.x1, data.y1);
      total_sum += 0.25 * f(data.x2, data.y1);
      total_sum += 0.25 * f(data.x1, data.y2);
      total_sum += 0.25 * f(data.x2, data.y2);

      result = total_sum * hx * hy;
      GetOutput() = result;
    }

    MPI_Bcast(&result, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank != 0) {
      GetOutput() = result;
    }

    return true;

  } catch (const std::exception &e) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0) {
      std::cerr << "Error in TimurAIntegralMPI::RunImpl: " << e.what() << std::endl;
    }
    return false;
  }
}

bool TimurAIntegralMPI::PostProcessingImpl() {
  return true;
}

}  // namespace timur_a_integral
