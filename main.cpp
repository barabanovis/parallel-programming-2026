#include <iostream>

#include "Matrix.h"
#include <filesystem>
#include <ctime>
#include <string>
using namespace std;

void print_results(int size, string file_path, double exec_time) {
	ofstream file(file_path, std::ios::app);
	if (!file.is_open()) {
		std::cerr << "Opening file error!\n";
		throw std::invalid_argument("Uncorrect file path!");
	}
	file << "Matrix " << size << " x " << size << " : " << exec_time << " sec.\n";
}

void print_matrix(Matrix<int>& C, string path) {
	ofstream file(path);
	file << C;
	file.close();
}

void experiment(int size) {
	string path_to_save = "../../../results/" + to_string(size);
	generate_int_matrix(size, size, path_to_save + "/Matrix_A.txt");
	generate_int_matrix(size, size, path_to_save + "/Matrix_B.txt");

	Matrix<int> A(path_to_save + "/Matrix_A.txt");
	Matrix<int> B(path_to_save + "/Matrix_B.txt");

	time_t start = clock();
	Matrix<int> C = A * B;
	time_t end = clock();
	double exec_time = (double)(end - start) / CLOCKS_PER_SEC;

	print_results(size, "../../../results/statistics.txt", exec_time);
	print_matrix(C, path_to_save + "/Matrix_C.txt");
}





int main() {
	ofstream file("../../../results/statistics.txt");
	file.close();

	for (int i = 100; i < 5000; i+=100) {
		experiment(i);
	}
}