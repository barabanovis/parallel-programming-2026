#include <iostream>

#include "Matrix.h"
#include <filesystem>
#include <ctime>

using namespace std;


void print_results(const Matrix<int>& C, ostream& os, double exec_time) {
	os << C.get_rows() << " " << C.get_columns() << '\n';
	os << C;
	os << '\n';
	os << "Execution time = " << exec_time << " seconds\n";
	os << "Task volume: 200x400 and 400x600\n";
}

int main() {
	generate_int_matrix(200, 400, "../../../Matrix_A.txt");
	generate_int_matrix(400, 600, "../../../Matrix_B.txt");

	Matrix<int> A("../../../Matrix_A.txt");
	Matrix<int> B("../../../Matrix_B.txt");

	time_t start = clock();
	Matrix<int> C = A * B;
	time_t end = clock();
	double exec_time = (double)(end - start) / CLOCKS_PER_SEC;

	ofstream result_file("../../../Matrix_C.txt");
	print_results(C, result_file, exec_time);
}