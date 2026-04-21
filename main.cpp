#include "mpi.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "Matrix.h"
#include <stdexcept>

using namespace std;


void print_matrix(Matrix<int>& C, string path) {
    // Create directory if needed
    size_t last_slash = path.find_last_of("/\\");
    if (last_slash != string::npos) {
        string dir_path = path.substr(0, last_slash);
        if (!dir_path.empty()) {
            createFileWithDirs(dir_path);
            cout << "Created directory: " << dir_path << "\n";
        }
    }

    // Write matrix to file
    ofstream file(path.c_str());
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
    double start = MPI_Wtime();

    int bcast_result = MPI_Bcast(B.data(), n * n, MPI_INT, 0, MPI_COMM_WORLD);
    if (bcast_result != MPI_SUCCESS) {
        cerr << "MPI_Bcast failed on rank " << rank << "\n";
        Experiment exp = { n, num_processes, -1.0 };
        return exp;
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
        Experiment exp = { n, num_processes, -1.0 };
        return exp;
    }

    int scatterv_result = MPI_Scatterv(A.data(), &send_counts[0], &displs[0], MPI_INT,
        &local_A[0], local_rows * n, MPI_INT, 0, MPI_COMM_WORLD);
    if (scatterv_result != MPI_SUCCESS) {
        cerr << "MPI_Scatterv failed on rank " << rank << "\n";
        Experiment exp = { n, num_processes, -1.0 };
        return exp;
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

    int gatherv_result = MPI_Gatherv(&local_result[0], local_rows * n, MPI_INT,
        (rank == 0 ? &flat_result[0] : NULL), &send_counts[0], &displs[0], MPI_INT,
        0, MPI_COMM_WORLD);
    if (gatherv_result != MPI_SUCCESS) {
        cerr << "MPI_Gatherv failed on rank " << rank << "\n";
        Experiment exp = { n, num_processes, -1.0 };
        return exp;
    }

    if (rank == 0) {
        for (size_t i = 0; i < flat_result.size(); i++) {
            A.data()[i] = flat_result[i];
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double stop = MPI_Wtime();

    Experiment exp = { n, num_processes, stop - start };
    return exp;
}

int parse_matrix_size(int argc, char** argv) {
    int matrix_size;
    if (argc < 2) {
        matrix_size = 100;
        cout << "Matrix size not specified. Using default value: "
            << matrix_size << '\n';
    }
    else {
        char* endptr;
        matrix_size = strtol(argv[1], &endptr, 10);
        if (*endptr != '\0' || matrix_size <= 0) {
            cerr << "Error: matrix size must be a positive number. "
                << "Using default value: 100" << '\n';
            matrix_size = 100;
        }
    }
    cout << "Using matrix size: " << matrix_size << '\n';
    return matrix_size;
}

void save_results(string file_path, Experiment exp) {
    // Create directory if needed
    size_t last_slash = file_path.find_last_of("/\\");
    if (last_slash != string::npos) {
        string dir_path = file_path.substr(0, last_slash);
        if (!dir_path.empty()) {
            createFileWithDirs(dir_path);
        }
    }

    ofstream res_file(file_path.c_str(), ios::app);
    if (res_file.is_open()) {
        res_file << exp.matrix_size << "," << exp.processes_num << "," << exp.exec_time << "\n";
        res_file.close();
        cout << "Results saved to " << file_path << "\n";
    }
    else {
        cerr << "Error: could not open results file for writing\n";
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, num_processes;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    Matrix<int> A, B;
    int matrix_size = 0;

    if (rank == 0) {
        matrix_size = parse_matrix_size(argc, argv);

        cout << "Generating matrices of size " << matrix_size << "x" << matrix_size << "...\n";
        A = generate_int_matrix(matrix_size, matrix_size);  // ← без объявления нового типа
        B = generate_int_matrix(matrix_size, matrix_size);  // ← без объявления нового типа

        cout << "Matrices generated and loaded by process 0\n";
    }

    // Рассылаем размер всем
    MPI_Bcast(&matrix_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Все процессы выделяют память под матрицы
    if (rank != 0) {
        A = Matrix<int>(matrix_size, matrix_size);
        B = Matrix<int>(matrix_size, matrix_size);
    }

    // Все процессы участвуют в широковещательной рассылке
    MPI_Bcast(A.data(), matrix_size * matrix_size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(B.data(), matrix_size * matrix_size, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        cout << "All processes have received matrices. Starting parallel multiplication...\n";
    }

    Experiment exp = MatrixMultiplicationMPI(A, B, rank, num_processes);

    if (rank == 0) {
        cout << "Matrix multiplication completed in " << exp.exec_time << " seconds\n";
    }

    MPI_Finalize();
    return 0;
}