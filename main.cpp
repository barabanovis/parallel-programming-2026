#include <iostream>
#include "mpi.h"
#include "Matrix.h"
#include <filesystem>
#include <ctime>
#include <string>

using namespace std;

void print_results(string file_path, size_t size, vector<double> exp_results) {
	ofstream file(file_path, std::ios::app);
	if (!file.is_open()) {
		std::cerr << "Opening file error!\n";
		throw std::invalid_argument("Uncorrect file path!");
	}

	file << size << ", ";

	for (auto u : exp_results) {
		file << u << ", ";
	}
	file << '\n';
}

void print_matrix(Matrix<int>& C, string path) {
	ofstream file(path);
	file << C;
	file.close();
}

double experiment(int size,size_t num_threads) {
	string path_to_save = "../../../results/threads_" + to_string(num_threads) + "/" + to_string(size);
	double exec_time = 0;
	for (int tryy = 1; tryy <= 5; tryy++) {
		string save_path = path_to_save + +"/exp_" + to_string(tryy) + "/";
		generate_int_matrix(size, size, save_path+ "Matrix_A.txt");
		generate_int_matrix(size, size, save_path + "Matrix_B.txt");

		Matrix<int> A(save_path + "Matrix_A.txt");
		Matrix<int> B(save_path + "Matrix_B.txt");

		time_t start = clock();
		Matrix<int> C = A.matr_multiply(B,num_threads);
		time_t end = clock();

		print_matrix(C, save_path + "Matrix_C.txt");
		exec_time += (double)(end - start) / CLOCKS_PER_SEC;
	}
	exec_time /= 5;

	return exec_time;
}





int main() {
	/*
	string result_file_path = "../../../results/statistics.csv";

	ofstream file(result_file_path);
	file.close();
	ofstream file(result_file_path);
	file << "size,1_flow,2_flows,3_flows,4_flows,5_flows,6_flows,7_flows,8_flows,9_flows,10_flows,";
	file.close();
	for (int size = 100; size < 2000; size += 100) {
		vector<double> exp_results(10);
		for (size_t num_threads = 1; num_threads <= 10; ++num_threads) {
			exp_results[num_threads - 1] = experiment(size, num_threads);
		}
		print_results(result_file_path, size, exp_results);
	}
	*/

}