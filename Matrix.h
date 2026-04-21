#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>

template <typename T>
class Matrix {
private:
	size_t _rows;
	size_t _columns;
	T* _data;
public:
	Matrix();
	Matrix(const Matrix<T>& copy);
	Matrix(size_t rows, size_t columns);

	~Matrix();

	T* data() const;
	size_t get_rows() const;
	size_t get_columns() const;

	Matrix<T>& operator=(const Matrix<T>& rhs);
	T& operator()(size_t row, size_t column);
	const T operator()(size_t row, size_t column) const;

	Matrix<T> matr_multiply(const Matrix<T>& rhs, size_t num_threads) const;
};

template<typename T>
T* Matrix<T>::data() const {
	return _data;
}

template<typename T>
Matrix<T>::Matrix(const Matrix<T>& copy) {
	_rows = copy.get_rows();
	_columns = copy.get_columns();
	_data = new T[_rows * _columns];

	for (size_t i = 0; i < get_rows()*get_columns(); ++i) {
		_data[i] = copy._data[i];
	}
}

template<typename T>
Matrix<T>& Matrix<T>::operator=(const Matrix<T>& rhs) {
	delete[] _data;

	_rows = rhs.get_rows();
	_columns = rhs.get_columns();

	_data = new T[_rows * _columns];
	for (size_t i = 0; i < _rows * _columns; i++) {
		_data[i] = rhs._data[i];
	}
	return *this;
}

template<typename T>
Matrix<T>::Matrix() : _data(NULL) {
	_rows = 0;
	_columns = 0;
}

template<typename T>
Matrix<T>::Matrix(size_t rows, size_t columns) : _rows(rows), _columns(columns) {
	_data = new T[_rows * _columns];
}

template<typename T>
Matrix<T>::~Matrix() {
	delete[] _data;
}

template<typename T>
size_t Matrix<T>::get_rows() const {
	return _rows;
}

template<typename T>
size_t Matrix<T>::get_columns() const {
	return _columns;
}

template<typename T>
T& Matrix<T>::operator()(size_t row, size_t column) {
	return _data[row * _columns + column];
}

template<typename T>
const T Matrix<T>::operator()(size_t row, size_t column) const {
	return _data[row * _columns + column];
}

template<typename T>
Matrix<T> Matrix<T>::matr_multiply(const Matrix<T>& rhs, size_t num_threads) const {
	if (get_columns() != rhs.get_rows()) {
		throw std::runtime_error("Matricies cannot be multiplicated!");
	}

	Matrix<T> result(get_rows(), rhs.get_columns());

	for (int i = 0; i < (int)result.get_rows(); ++i) {
		for (int j = 0; j < (int)result.get_columns(); ++j) {
			T sum = 0;
			for (int s = 0; s < (int)get_columns(); ++s) {
				sum += (*this)(i, s) * rhs(s, j);
			}
			result(i, j) = sum;
		}
	}
	return result;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const Matrix<T>& matr) {
	for (size_t i = 0; i < matr.get_rows(); ++i) {
		for (size_t j = 0; j < matr.get_columns(); ++j) {
			os << matr(i, j) << " ";
		}
		os << "\n";
	}
	return os;
}


void createFileWithDirs(const std::string& file_path) {
	std::ofstream file(file_path.c_str(), std::ios::out | std::ios::trunc);
	if (!file.is_open()) {
		throw std::runtime_error("Cannot open file: " + file_path +
			" (directory may not exist or insufficient permissions)");
	}
	file.close();
	std::cout << "File created: " << file_path << std::endl;
}

void generate_int_matrix(size_t rows, size_t columns, std::string file_path) {
	srand(time(NULL));

	createFileWithDirs(file_path);
	std::ofstream file;
	file.open(file_path.c_str(), std::ios::out | std::ios::trunc);
	if (!file.is_open()) {
		throw std::invalid_argument("Uncorrect file path!");
	}

	file << rows << " " << columns << '\n';
	for (size_t i = 0; i < rows; i++) {
		for (size_t j = 0; j < columns; j++) {
			file << (rand() % 100 + 1) << " ";
		}
		file << '\n';
	}
	file.close();
}

Matrix<int> generate_int_matrix(size_t rows, size_t columns) {
	srand(time(NULL));

	Matrix<int> M(rows, columns);
	for (size_t i = 0; i < rows; ++i) {
		for (size_t j = 0; j < columns; j++) {
			M(i, j) = rand() % 100 + 1;
		}
	}
	return M;
}

#endif // MATRIX_H