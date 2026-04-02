#include <iostream>

#include "Matrix.h"
#include <filesystem>
#include <ctime>

using namespace std;


int main() {
	generate_int_matrix(200, 400, "../../../Matrix_A.txt");
	generate_int_matrix(400, 600, "../../../Matrix_B.txt");

	Matrix<int> A("../../../Matrix_A.txt");
	Matrix<int> B("../../../Matrix_B.txt");
	
	time_t start = clock();
	Matrix<int> C = A * B;
	time_t end = clock();


	ofstream result_file("../../../Matrix_C.txt");
	result_file << C;
	result_file << '\n';
	result_file << "Execution time = " << end - start << "\n";
	result_file << "Task volume: 200x400 and 400x600\n";
}