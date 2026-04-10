#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <random>

namespace fs = std::filesystem;

template <typename T>
class Matrix {
private:
	size_t _rows;
	size_t _columns;
	T* _data;
public:
	Matrix(size_t rows, size_t columns);
	Matrix(std::string file_path);

	~Matrix();

	size_t get_rows() const;
	size_t get_columns() const;

	T& operator()(size_t row, size_t column);
	const T operator()(size_t row, size_t column) const;

	Matrix<T> operator*(const Matrix<T>& rhs) const;
};

template<typename T>
Matrix<T>::Matrix(size_t rows, size_t columns) :_rows(rows), _columns(columns) {
	_data = new T[_rows * _columns];
}

template<typename T>
Matrix<T>::Matrix(std::string file_path) {
	std::ifstream matrix_file(file_path);

	if (!matrix_file.is_open()) {
		throw std::invalid_argument("Incorrect file path or cannot open file!");
	}

	int rows, columns;
	matrix_file >> rows >> columns;

	if (rows <= 0 || columns <= 0) {
		throw std::invalid_argument("Matrix dimensions must be positive!");
	}

	
	_rows = rows;
	_columns = columns;
	_data = new T[rows * columns];

	for (size_t i = 0; i < rows; ++i) {
		for (size_t j = 0; j < columns; ++j) {
			if (!(matrix_file >> (*this)(i, j))) {
				delete[] _data;  
				throw std::runtime_error("Failed to read matrix element at position ("
					+ std::to_string(i) + ", " + std::to_string(j) + ")");
			}
		}
	}

	matrix_file.close();
}


template<typename T>
Matrix<T>::~Matrix() {
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
const T Matrix<T>::operator()(size_t row, size_t column) const {
	return _data[row * _columns + column];
}

template<typename T>
Matrix<T> Matrix<T>::operator*(const Matrix<T>& rhs) const {
	if (get_columns() != rhs.get_rows()) {
		throw std::runtime_error("Matricies cannot be multiplicated!");
	}
	Matrix<T> result(get_rows(), rhs.get_columns());
	for (size_t i = 0; i < result.get_rows(); ++i) {
		for (size_t j = 0; j < result.get_columns(); ++j) {
			result(i, j) = 0;
			for (size_t s = 0; s < get_columns(); ++s) {
				result(i, j) += (*this)(i, s) * rhs(s, j);
			}
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
	try {
		// Извлекаем путь к директории из полного пути к файлу
		fs::path path(file_path);
		fs::path dir_path = path.parent_path();

		// Если директория не пустая и не существует — создаём все промежуточные папки
		if (!dir_path.empty() && !fs::exists(dir_path)) {
			fs::create_directories(dir_path);
			std::cout << "Created directory: " << dir_path << std::endl;
		}

		// Создаём/открываем файл для записи
		std::ofstream file(file_path, std::ios::out | std::ios::trunc);
		if (!file.is_open()) {
			throw std::runtime_error("Cannot open file: " + file_path);
		}
		file.close();

		std::cout << "File created: " << file_path << std::endl;
	}
	catch (const fs::filesystem_error& e) {
		throw std::runtime_error("Ошибка файловой системы: " + std::string(e.what()));
	}
}



void generate_int_matrix(size_t rows, size_t columns, std::string file_path) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(1, 100);

	createFileWithDirs(file_path);
	std::ofstream file;
	file.open(file_path, std::ios::out | std::ios::trunc);
	if (!file.is_open()) {
		throw std::invalid_argument("Uncorrect file path!");
	}


	file << rows << " " << columns << '\n';
	for (size_t i = 0; i < rows; i++) {
		for (size_t j = 0; j < columns; j++) {
			file << dist(gen) << " ";
		}
		file << '\n';
	}
}
#endif MATRIX_H




