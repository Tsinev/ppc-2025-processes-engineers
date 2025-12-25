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
double ComputeTrapezoidal2D(const TaskData &data, int rank, int size, const Func &f) {
  double n_steps = static_cast<double>(data.n_steps);
  double hx = (data.x2 - data.x1) / n_steps;
  double hy = (data.y2 - data.y1) / n_steps;

  int total_points_x = data.n_steps + 1;

  int points_per_proc = total_points_x / size;
  int remainder = total_points_x % size;

  int start_i = rank * points_per_proc + std::min(rank, remainder);
  int end_i = start_i + points_per_proc + (rank < remainder ? 1 : 0);

  if (start_i >= end_i) {
    return 0.0;
  }

  double local_sum = 0.0;

  for (int i = start_i; i < end_i; ++i) {
    double x = data.x1 + i * hx;

    double weight_x;
    if (i == 0 || i == data.n_steps) {
      weight_x = 0.5;
    } else {
      weight_x = 1.0;
    }

    for (int j = 0; j <= data.n_steps; ++j) {
      double y = data.y1 + j * hy;

      double weight_y;
      if (j == 0 || j == data.n_steps) {
        weight_y = 0.5;
      } else {
        weight_y = 1.0;
      }

      local_sum += f(x, y) * weight_x * weight_y;
    }
  }

  return local_sum * hx * hy;
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

    double local_result = 0.0;

    switch (data.func_id) {
      case 0:
        local_result = ComputeTrapezoidal2D(data, rank, size, [](double x, double y) { return x + y; });
        break;

      case 1:
        local_result = ComputeTrapezoidal2D(data, rank, size, [](double x, double y) { return x * x + y * y; });
        break;

      case 2:
        local_result =
            ComputeTrapezoidal2D(data, rank, size, [](double x, double y) { return std::sin(x) * std::cos(y); });
        break;

      case 3:
        local_result = ComputeTrapezoidal2D(data, rank, size, [](double x, double y) { return std::exp(x + y); });
        break;

      case 4:
        local_result =
            ComputeTrapezoidal2D(data, rank, size, [](double x, double y) { return std::sqrt(x * x + y * y); });
        break;

      case 5:
        local_result = ComputeTrapezoidal2D(data, rank, size, [](double x, double y) {
          (void)x;
          (void)y;
          return 1.0;
        });
        break;

      default:
        local_result = ComputeTrapezoidal2D(data, rank, size, [](double x, double y) {
          (void)x;
          (void)y;
          return 1.0;
        });
        break;
    }

    double global_result = 0.0;
    MPI_Reduce(&local_result, &global_result, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    MPI_Bcast(&global_result, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    GetOutput() = global_result;

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
