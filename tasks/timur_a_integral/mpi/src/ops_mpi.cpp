#include "timur_a_integral/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cmath>

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
// Константа для минимального количества шагов на процесс
const int MIN_STEPS_PER_PROCESS = 10;

template <typename Func>
double RunKernel(const TaskData &data, int rank, int size, const Func &f) {
  double hx = (data.x2 - data.x1) / data.n_steps;
  double hy = (data.y2 - data.y1) / data.n_steps;

  // Определяем, сколько процессов действительно нужно
  int total_nodes_x = data.n_steps + 1;

  // Если шагов слишком мало на процесс, уменьшаем количество активных процессов
  int active_processes = std::min(size, std::max(1, total_nodes_x / MIN_STEPS_PER_PROCESS));
  if (active_processes == 0) {
    active_processes = 1;
  }

  // Если текущий процесс неактивен, возвращаем 0
  if (rank >= active_processes) {
    return 0.0;
  }

  // Перераспределяем работу только между активными процессами
  int count = total_nodes_x / active_processes;
  int remainder = total_nodes_x % active_processes;

  int start_i = (rank * count) + std::min(rank, remainder);
  int end_i = start_i + count + (rank < remainder ? 1 : 0);

  double local_sum = 0.0;

  // Вычисляем только если есть работа
  if (start_i < end_i) {
    for (int i = start_i; i < end_i; ++i) {
      double x = data.x1 + (i * hx);
      double weight_x = (i == 0 || i == data.n_steps) ? 0.5 : 1.0;

      for (int j = 0; j <= data.n_steps; ++j) {
        double y = data.y1 + (j * hy);
        double weight_y = (j == 0 || j == data.n_steps) ? 0.5 : 1.0;

        local_sum += f(x, y) * weight_x * weight_y;
      }
    }
  }

  return local_sum * hx * hy;
}
}  // namespace

bool TimurAIntegralMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  TaskData data{};
  if (rank == 0) {
    data = GetInput();
  }

  // Рассылаем данные всем процессам
  MPI_Bcast(&data.n_steps, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&data.func_id, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&data.x1, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(&data.x2, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(&data.y1, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(&data.y2, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  double local_result = 0.0;

  // Определяем активные процессы
  int total_nodes_x = data.n_steps + 1;
  int active_processes = std::min(size, std::max(1, total_nodes_x / MIN_STEPS_PER_PROCESS));
  if (active_processes == 0) {
    active_processes = 1;
  }

  // Логирование для отладки (только для первых нескольких процессов)
  if (rank == 0) {
    std::cout << "DEBUG: n_steps=" << data.n_steps << ", total_nodes=" << total_nodes_x << ", total_processes=" << size
              << ", active_processes=" << active_processes << std::endl;
  }

  // Синхронизируем все процессы перед началом вычислений
  MPI_Barrier(MPI_COMM_WORLD);

  // Вычисляем только если процесс активен
  if (rank < active_processes) {
    switch (data.func_id) {
      case 0:
        local_result = RunKernel(data, rank, size, [](double x, double y) { return x + y; });
        break;
      case 1:
        local_result = RunKernel(data, rank, size, [](double x, double y) { return (x * x) + (y * y); });
        break;
      case 2:
        local_result = RunKernel(data, rank, size, [](double x, double y) { return std::sin(x) * std::cos(y); });
        break;
      case 3:
        local_result = RunKernel(data, rank, size, [](double x, double y) { return std::exp(x + y); });
        break;
      case 4:
        local_result = RunKernel(data, rank, size, [](double x, double y) { return std::sqrt((x * x) + (y * y)); });
        break;
      default:
        local_result = RunKernel(data, rank, size, [](double x, double y) {
          (void)x;
          (void)y;
          return 1.0;
        });
        break;
    }
  }

  // Синхронизируем перед операцией reduce
  MPI_Barrier(MPI_COMM_WORLD);

  double global_result = 0.0;
  MPI_Reduce(&local_result, &global_result, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  // Рассылаем результат всем процессам
  MPI_Bcast(&global_result, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  // Записываем результат (все процессы получают одинаковый результат)
  GetOutput() = global_result;

  // Финальная синхронизация
  MPI_Barrier(MPI_COMM_WORLD);

  return true;
}

bool TimurAIntegralMPI::PostProcessingImpl() {
  return true;
}

}  // namespace timur_a_integral
