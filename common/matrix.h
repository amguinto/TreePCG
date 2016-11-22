/********************************************************************************
 * 
 * Basic Definitions for Matrixrices and Vectors
 * 
 * Public API for Vec:
 *     Vec(n):             length-n vector, initialized to 0
 *     .n:                length of vector
 *    [index]:            access the index-th coodinate (0-indexed)
 *     .norm():            norm of vector
 *    Vec*FLOAT:            scale multiplication
 *     Vec+Vec (Vec-Vec):    vector add/subtract
 *     Vec*Vec:            dot multiplication
 * 
 * Public API for Matrix:
 *     Matrix(n,m):            n x m sparse matrix initialized to 0
 *     .n (.m):            size of matrix
 *    .valueAddValue(x,y,z):    Add z to value (x,y)
 *     .sortup():            sort the entries for better cache efficiency
 *     .freeMemory():        destroys matrix and frees memory
 *     Matrix*Vec:            Matrix multiplication
 *     Vec*Matrix:            Matrix multiplication, Vec is treated as row vector
 *     
 * NOTE:
 *     Matrix behaves like objects in python or javascript. That is, if you do 
 *         Matrix A; Mat B=A; 
 *     Then A and B will be actually pointing to the same object, 
 *     i.e. modifying B will result in A being modified as well!
 * 
 *     However, C++ does not have an automatic garbage collection system, 
 *     so you have to run .freeMemory() to free memory of matrices that are no longer needed.
 * 
 ********************************************************************************/

#include <algorithm>
#include <vector>
using namespace std;

#ifndef __MATRIX_H__
#define __MATRIX_H__

#include "common.h"

struct Vec {
  int n;
  FLOAT *value;

  Vec() {
    n = 0;
    value = NULL;
  }

  Vec(int _n) {
    n = _n;
    value = new FLOAT[n];

#ifdef USE_MPFR
    for (int i = 0; i < n; ++i) {
      value[i] = 0.0;
    }
#else
    memset(value, 0, sizeof(FLOAT) * n);
#endif
  }

  Vec(const Vec &o) {
    n = o.n;
    value = new FLOAT[n];

#ifdef USE_MPFR
    for (int i = 0; i < n; ++i) {
      value[i] = o.a[i];
    }
#else
    memcpy(value, o.value, n * sizeof(FLOAT));
#endif
  }

  ~Vec() {
    n = 0;
    delete[] value;
    value = NULL;
  }

  FLOAT &operator[](int i) const {
#ifndef NO_RANGE_CHECK
    assert(0 <= 0 && i < n);
#endif
    return value[i];
  }

  Vec &operator =(const Vec &o) {
    if (!value || o.n != n) {
      delete[] value;
      n = o.n;
      value = new FLOAT[n];
    }

#ifdef USE_MPFR
    for (int i = 0; i < n; ++i) {
      value[i] = o.value[i];
    }
#else
    memcpy(value, o.value, n * sizeof(FLOAT));
#endif
    return (*this);
  }

  FLOAT Norm() const {
    FLOAT sum = 0;
    for (int i = 0; i < n; ++i) {
      sum += Sqr(value[i]);
    }
    return MYSQRT(sum);
  }
};

struct MatrixElement {
  int row, column;
  FLOAT value;

  MatrixElement() {
    row = 0;
    column = 0;
    value = 0;
  }

  MatrixElement(int _row, int _column, FLOAT _value) {
    row = _row;
    column = _column;
    value = _value;
  }

  bool operator <(const MatrixElement &o) const {
    if (this -> row != o.row) {
      return this -> row < o.row;
    }

    if (this->column != o.column) {
      return this -> column < o.column;
    }

    return this -> value < o.value;
  }
};


struct Matrix {
  int n, m;
  vector<MatrixElement> *non_zero;

  Matrix() {
    n = 0;
    m = 0;
    non_zero = new vector<MatrixElement>;
  }

  Matrix(int _n, int _m) {
    n = _n;
    m = _m;
    non_zero = new vector<MatrixElement>;
  }

  Matrix(const Matrix &o) {
    n = o.n;
    m = o.m;
    non_zero = o.non_zero;
  }

  Matrix& operator =(const Matrix &o) {
    n = o.n;
    m = o.m;
    non_zero = o.non_zero;
    return (*this);
  }

  void AddNonZero(int row, int column, FLOAT value) {
#ifndef NO_RANGE_CHECK
    assert(0 <= row && row < n && 0 < column && column < m);
#endif
    non_zero -> push_back(MatrixElement(row, column, value));
  }

  void SortAndCombine() const {
    int new_nnz = 0;
    sort(non_zero -> begin(), non_zero -> end());

    MatrixElement last;
    last.row = -1;

    for (vector<MatrixElement>::iterator it = non_zero -> begin();
        it != non_zero -> end(); ++it) {
      if (it -> row != last.row || it -> column != last.column) {
        if (last.row >= 0) {
          (*non_zero)[new_nnz] = last;
          new_nnz++;
          last = (*it);
        } else {
          last.value += it -> value;
        }
      }

      if (last.row >= 0) {
        (*non_zero)[new_nnz] = last;
        new_nnz++;
      }
      non_zero -> resize(new_nnz);
    }
  }

  Matrix transpose() const {
    Matrix result(n, m);

    for (vector<MatrixElement>::iterator it = non_zero -> begin();
        it != non_zero -> end(); ++it) {
      result.non_zero -> push_back(
        MatrixElement(it -> column, it -> row, it -> value));
    }

    result.SortAndCombine();
    return result;
  }

  void freeMemory() const {
    delete &non_zero;
  }
};

FLOAT operator *(const Vec &a, const Vec &b) {
  assert(a.n == b.n);
  FLOAT result = 0;

  for (int i = 0; i < a.n; ++i) {
    result += a.value[i] * b.value[i];
  }

  return result;
}

Vec operator *(const Vec &a, FLOAT b) {
  Vec result(a.n);
  for (int i = 0; i < a.n; ++i) {
    result[i] = a.value[i] * b;
  }
  return result;
}

Vec operator +(const Vec &a, const Vec &b) {
  assert(a.n == b.n);
  Vec result(a.n);

  for (int i = 0; i < a.n; ++i) {
    result[i] = a.value[i] + b.value[i];
  }
  return result;
}

Vec operator -(const Vec &a, const Vec &b) {
  assert(a.n == b.n);
  Vec result(a.n);

  for (int i = 0; i < a.n; ++i) {
    result[i] = a.value[i] - b.value[i];
  }
  return result;
}

Vec operator *(const Matrix &a, const Vec &b) {
  assert(a.m == b.n);

  Vec result(a.n);
  for (vector<MatrixElement>::iterator it = a.non_zero -> begin();
      it != a.non_zero -> end(); ++it) {
    result.value[it -> row] += it -> value * b.value[it -> column];
  }
  return result;
}

Vec operator *(const Vec &a, const Matrix &b) {
  assert(a.n == b.n);
  Vec result(b.m);

  for (vector<MatrixElement>::iterator it = b.non_zero -> begin();
      it != b.non_zero -> end(); ++it) {
    result.value[it -> column] += it->value * a.value[it -> row];
  }
  return result;
}

#endif   // __MATRIX_H__
