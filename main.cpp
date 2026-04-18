#include <iostream>
#include "mpi.h"
#include "Matrix.h"
#include <filesystem>
#include <ctime>
#include <string>
#include <fstream>

using namespace std;
namespace fs = std::filesystem;

// Функция для чтения JSON файла (улучшенный парсинг)
string get_json_value(const string& json_path, const string& key) {
    ifstream file(json_path);
    if (!file.is_open()) {
        cerr << "Error: Could not open config file: " << json_path << "\n";
        return "";
    }

    string content, line;
    while (getline(file, line)) {
        content += line;
    }
    file.close();

    // Ищем ключ
    string search_key = "\"" + key + "\"";
    size_t key_pos = content.find(search_key);
    if (key_pos == string::npos) {
        cerr << "Error: Key not found: " << key << "\n";
        return "";
    }

    // Ищем двоеточие после ключа
    size_t colon_pos = content.find(":", key_pos);
    if (colon_pos == string::npos) {
        return "";
    }

    // Ищем открывающую кавычку значения
    size_t start_quote = content.find("\"", colon_pos + 1);
    if (start_quote == string::npos) {
        // Может быть значение без кавычек (число)
        size_t end_pos = content.find_first_of(",}\n", colon_pos + 1);
        if (end_pos != string::npos) {
            return content.substr(colon_pos + 1, end_pos - colon_pos - 1);
        }
        return "";
    }

    // Ищем закрывающую кавычку
    size_t end_quote = content.find("\"", start_quote + 1);
    if (end_quote == string::npos) {
        return "";
    }

    return content.substr(start_quote + 1, end_quote - start_quote - 1);
}

void print_matrix(Matrix<int>& C, string path) {
    // Создаем директорию, если её нет
    fs::path file_path(path);
    fs::path dir_path = file_path.parent_path();
    if (!dir_path.empty() && !fs::exists(dir_path)) {
        fs::create_directories(dir_path);
        cout << "Created directory: " << dir_path << "\n";
    }

    // Сохраняем матрицу
    ofstream file(path);
    if (file.is_open()) {
        file << C;
        file.close();
        cout << "Result matrix C saved to " << path << "\n";
    }
    else {
        cerr << "Error: could not save matrix C to " << path << "\n";
    }
}

struct Experiment {
    int matrix_size;
    int processes_num;
    double exec_time;
};

Experiment MatrixMultiplicationMPI(Matrix<int>& A, Matrix<int>& B, int rank, int num_processes) {
    int n = A.get_rows();

    if (num_processes > n) {
        if (rank == 0) {
            cerr << "Warning: More processes (" << num_processes
                << ") than rows (" << n << ")\n";
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    auto start = MPI_Wtime();

    int bcast_result = MPI_Bcast(B.data(), n * n, MPI_INT, 0, MPI_COMM_WORLD);
    if (bcast_result != MPI_SUCCESS) {
        cerr << "MPI_Bcast failed on rank " << rank << "\n";
        return { n, num_processes, -1.0 };
    }

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

    int local_rows = rows_count[rank];
    vector<int> local_A(local_rows * n);

    if (local_A.empty() && local_rows > 0) {
        cerr << "Memory allocation failed on rank " << rank << "\n";
        return { n, num_processes, -1.0 };
    }

    int scatterv_result = MPI_Scatterv(A.data(), send_counts.data(), displs.data(), MPI_INT,
        local_A.data(), local_rows * n, MPI_INT, 0, MPI_COMM_WORLD);
    if (scatterv_result != MPI_SUCCESS) {
        cerr << "MPI_Scatterv failed on rank " << rank << "\n";
        return { n, num_processes, -1.0 };
    }

    vector<int> local_result(local_rows * n, 0);

    for (int i = 0; i < local_rows; i++) {
        for (int k = 0; k < n; k++) {
            int aik = local_A[i * n + k];
            if (aik == 0) continue;
            for (int j = 0; j < n; j++) {
                local_result[i * n + j] += aik * B.data()[k * n + j];
            }
        }
    }

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
    // Читаем пути из JSON файла
    string config_path = "../../../paths.json";
    string work_path = get_json_value(config_path, "work_path");
    string results_dir = get_json_value(config_path, "results_dir");
    string matrix_a_file = get_json_value(config_path, "matrix_a_file");
    string matrix_b_file = get_json_value(config_path, "matrix_b_file");
    string matrix_c_file = get_json_value(config_path, "matrix_c_file");
    string statistics_file = get_json_value(config_path, "statistics_file");

    // Формируем полные пути
    string results_path = work_path + "/" + results_dir;
    string matrA_path = results_path + "/" + matrix_a_file;
    string matrB_path = results_path + "/" + matrix_b_file;
    string matrC_path = results_path + "/" + matrix_c_file;
    string statistics_path = results_path + "/" + statistics_file;

    MPI_Init(&argc, &argv);

    int rank, num_processes;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    Matrix<int> A, B, C;  // Добавлена матрица C для результата
    int matrix_size = 0;

    if (rank == 0) {
        matrix_size = parse_matrix_size(argc, argv);

        fs::path results_dir_path = results_path;
        if (!fs::exists(results_dir_path)) {
            fs::create_directories(results_dir_path);
        }

        cout << "Generating matrices of size " << matrix_size << "x" << matrix_size << "...\n";
        generate_int_matrix(matrix_size, matrix_size, matrA_path);
        generate_int_matrix(matrix_size, matrix_size, matrB_path);

        A = Matrix<int>(matrA_path);
        B = Matrix<int>(matrB_path);

        cout << "Matrices generated and loaded by process 0\n";
    }

    MPI_Bcast(&matrix_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        A = Matrix<int>(matrix_size, matrix_size);
        B = Matrix<int>(matrix_size, matrix_size);
    }

    MPI_Bcast(A.data(), matrix_size * matrix_size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(B.data(), matrix_size * matrix_size, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        cout << "All processes have received matrices. Starting parallel multiplication...\n";
    }

    Experiment exp = MatrixMultiplicationMPI(A, B, rank, num_processes);

    if (rank == 0) {
        // Сохраняем результаты
        save_results(statistics_path, exp);

        // Сохраняем результирующую матрицу C (A после умножения содержит результат)
        print_matrix(A, matrC_path);

        cout << "Matrix multiplication completed in " << exp.exec_time << " seconds\n";
        cout << "Result matrix C saved to " << matrC_path << "\n";
    }

    MPI_Finalize();
    return 0;
}