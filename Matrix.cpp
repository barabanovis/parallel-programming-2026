#include "Matrix.h"

using namespace std;

template<typename T>
Matrix<T>::Matrix(size_t rows, size_t columns):_rows(rows),_columns(columns) {
	_data = new T[_rows * _columns];
}

template<typename T>
Matrix<T>::~Matrix(){
	delete _data;
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
const T Matrix<T>::operator()(size_t row, size_t column) const{
	return _data[row * _columns + column];
}

template<typename T>
Matrix<T> Matrix<T>::operator*(const Matrix<T>& rhs) const{
	if (get_columns() != rhs.get_rows()) {
		throw std::runtime_error("Matricies cannot be multiplicated!")
	}
	Matrix<T> result(get_rows(), rhs.get_columns());
	for (size_t i = 0; i < result.get_rows(); ++i) {
		for (size_t j = 0; j < result.get_columns(); ++j) {
			result(i, j) = 0;
			for (size_t s = 0; s < get_columns(); ++s) {
				result(i, j) += *this(i, s) * rhs(s, j);
			}
		}
	}
	return result;
}