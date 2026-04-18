#include <iostream>
#include "mpi.h"
#include "Matrix.h"
#include <filesystem>
#include <ctime>
#include <string>

using namespace std;

void print_matrix(Matrix<int>& C, string path) {
    ofstream file(path);
    file << C;
    file.close();
}

struct Experiment {
    int matrix_size;
    int processes_num;
    double exec_time;
};

Experiment MatrixMultiplicationMPI(Matrix<int>& A, Matrix<int>& B, int rank, int num_processes) {
    int n = A.get_rows();  // Размер матрицы

    // Проверка: не больше ли процессов, чем строк
    if (num_processes > n) {
        if (rank == 0) {
            cerr << "Warning: More processes (" << num_processes
                << ") than rows (" << n << ")\n";
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    auto start = MPI_Wtime();

    // 1. Рассылка матрицы B всем процессам
    int bcast_result = MPI_Bcast(B.data(), n * n, MPI_INT, 0, MPI_COMM_WORLD);
    if (bcast_result != MPI_SUCCESS) {
        cerr << "MPI_Bcast failed on rank " << rank << "\n";
        return { n, num_processes, -1.0 };
    }

    // 2. Распределение строк матрицы A
    int rows_per_process = n / num_processes;
    int remainder = n % num_processes;

    vector<int> rows_count(num_processes);
    vector<int> send_counts(num_processes);
    vector<int> displs(num_processes);

    int current_row = 0;
    for (int i = 0; i < num_processes; i++) {
        rows_count[i] = rows_per_process + (i < remainder ? 1 : 0);
        send_counts[i] = rows_count[i] * n;
        displs[i] = current_row * n;
        current_row += rows_count[i];
    }

    // 3. Локальные данные для каждого процесса
    int local_rows = rows_count[rank];
    vector<int> local_A(local_rows * n);

    // Проверка размера локального буфера
    if (local_A.empty() && local_rows > 0) {
        cerr << "Memory allocation failed on rank " << rank << "\n";
        return { n, num_processes, -1.0 };
    }

    // 4. Рассылка строк матрицы A
    int scatterv_result = MPI_Scatterv(A.data(), send_counts.data(), displs.data(), MPI_INT,
        local_A.data(), local_rows * n, MPI_INT, 0, MPI_COMM_WORLD);
    if (scatterv_result != MPI_SUCCESS) {
        cerr << "MPI_Scatterv failed on rank " << rank << "\n";
        return { n, num_processes, -1.0 };
    }

    // 5. Параллельное вычисление (каждый процесс считает свою часть)
    vector<int> local_result(local_rows * n, 0);

    // Оптимизированный порядок умножения (ikj)
    for (int i = 0; i < local_rows; i++) {
        for (int k = 0; k < n; k++) {
            int aik = local_A[i * n + k];
            if (aik == 0) continue;  // Пропуск нулей
            for (int j = 0; j < n; j++) {
                local_result[i * n + j] += aik * B.data()[k * n + j];
            }
        }
    }

    // 6. Сбор результатов на процессе 0
    vector<int> flat_result;
    if (rank == 0) {
        flat_result.resize(n * n);
    }

    int gatherv_result = MPI_Gatherv(local_result.data(), local_rows * n, MPI_INT,
        flat_result.data(), send_counts.data(), displs.data(), MPI_INT,
        0, MPI_COMM_WORLD);
    if (gatherv_result != MPI_SUCCESS) {
        cerr << "MPI_Gatherv failed on rank " << rank << "\n";
        return { n, num_processes, -1.0 };
    }

    // 7. Копируем результат обратно в A на процессе 0
    if (rank == 0) {
        for (size_t i = 0; i < flat_result.size(); i++) {
            A.data()[i] = flat_result[i];
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    auto stop = MPI_Wtime();

    return { n, num_processes, stop - start };
}

int parse_matrix_size(int& argc, char**& argv) {
    int matrix_size;
    if (argc < 2) {
        matrix_size = 100;
        std::cout << "Matrix size not specified. Using default value: "
            << matrix_size << '\n';
    }
    else {
        try {
            matrix_size = std::stoi(argv[1]);
            if (matrix_size <= 0) {
                std::cerr << "Error: matrix size must be a positive number. "
                    << "Using default value: 100" << '\n';
                matrix_size = 100;
            }
        }
        catch (const std::invalid_argument& e) {
            std::cerr << "Error: first argument must be a number. "
                << "Using default value: 100" << '\n';
            matrix_size = 100;
        }
        catch (const std::out_of_range& e) {
            std::cerr << "Error: number is too large. "
                << "Using default value: 100" << '\n';
            matrix_size = 100;
        }
    }
    std::cout << "Using matrix size: " << matrix_size << '\n';
    return matrix_size;
}

void save_results(string file_path, Experiment exp) {
    // Создаем директорию если не существует
    fs::path path(file_path);
    fs::path dir_path = path.parent_path();
    if (!dir_path.empty() && !fs::exists(dir_path)) {
        fs::create_directories(dir_path);
    }

    std::ofstream res_file(file_path, std::ios::app);
    if (res_file.is_open()) {
        res_file << exp.matrix_size << "," << exp.processes_num << "," << exp.exec_time << "\n";
        res_file.close();
        std::cout << "Results saved to " << file_path << "\n";
    }
    else {
        std::cerr << "Error: could not open results file for writing\n";
    }
}

int main(int argc, char** argv) {
    string work_path = "E:/working/parallel-programming-2026";
    MPI_Init(&argc, &argv);

    int rank, num_processes;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    Matrix<int> A, B;  // Пустые матрицы
    int matrix_size = 0;

    // ТОЛЬКО ПРОЦЕСС 0: создает файлы, генерирует и загружает матрицы
    if (rank == 0) {
        matrix_size = parse_matrix_size(argc, argv);

        string matrA_path = work_path+"/results/matrA.txt";
        string matrB_path = work_path + "/results/matrB.txt";

        // Создаем директорию для результатов
        fs::path results_dir = work_path + "/results";
        if (!fs::exists(results_dir)) {
            fs::create_directories(results_dir);
        }

        // Генерируем матрицы
        cout << "Generating matrices of size " << matrix_size << "x" << matrix_size << "...\n";
        generate_int_matrix(matrix_size, matrix_size, matrA_path);
        generate_int_matrix(matrix_size, matrix_size, matrB_path);

        // Загружаем матрицы
        A = Matrix<int>(matrA_path);
        B = Matrix<int>(matrB_path);

        cout << "Matrices generated and loaded by process 0\n";
    }

    // Рассылаем размер матрицы всем процессам
    MPI_Bcast(&matrix_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Для не-нулевых процессов создаем пустые матрицы нужного размера
    if (rank != 0) {
        A = Matrix<int>(matrix_size, matrix_size);
        B = Matrix<int>(matrix_size, matrix_size);
    }

    // Рассылаем данные матриц (только после того, как процесс 0 их загрузил)
    MPI_Bcast(A.data(), matrix_size * matrix_size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(B.data(), matrix_size * matrix_size, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        cout << "All processes have received matrices. Starting parallel multiplication...\n";
    }

    // ВСЕ РАСПАРАЛЛЕЛИВАНИЕ ТОЛЬКО ЗДЕСЬ!
    Experiment exp = MatrixMultiplicationMPI(A, B, rank, num_processes);

    // Сохраняем результаты (только процесс 0)
    if (rank == 0) {
        save_results(work_path + "/results/statistics.csv", exp);
        cout << "Matrix multiplication completed in " << exp.exec_time << " seconds\n";
    }

    MPI_Finalize();
    return 0;
}