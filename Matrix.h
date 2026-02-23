#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>

template <typename T>
class Matrix {
private:
	size_t _rows;
	size_t _columns;
	T* _data;
public:
	Matrix(size_t rows, size_t columns);

	~Matrix();

	size_t get_rows() const;
	size_t get_columns() const;

	T& operator()(size_t row, size_t column);
	const T operator()(size_t row, size_t column) const;

	Matrix<T> operator*(const Matrix<T>& rhs) const;
};



#endif MATRIX_H




