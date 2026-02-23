#include <iostream>

#include "Matrix.h"

using namespace std;


int main() {
	Matrix<int> A(3, 2);

	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 2; j++) {
			cin >> A(i, j);
		}
	}

	Matrix<int> B(2, 5);
	for (size_t i = 0; i < 2; i++) {
		for (size_t j = 0; j < 5; j++) {
			cin >> B(i, j);
		}
	}

	Matrix<int> C = A * B;
	for (size_t i = 0; i < C.get_rows(); i++) {
		cout << '\n';
		for (size_t j = 0; j < C.get_columns(); j++) {
			cout << C(i, j) << " ";
		}
	}
}