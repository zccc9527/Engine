#pragma once
#include "vector.h"
#include <assert.h>

//æÿ’Ûƒ£∞Â¿‡
template<typename T, int Rows, int Cols>
struct Matrix
{
	T data[Rows][Cols];

	Matrix()
	{
		memset((void*)data, 0, sizeof(T) * Rows * Cols);
	}
	T* operator[](const int rowIndex)
	{
		assert(rowIndex < Rows);
		return data[rowIndex];
	}

	int GetRowNum() const { return Rows; }
	int GetColNum() const { return Cols; }
};

template<typename T>
struct Matrix3x3 : Matrix<T, 3, 3>
{
	Matrix3x3() { IdentityMatrix(); }

	void IdentityMatrix()
	{
		this->data[0][0] = T{ 1 }; this->data[0][1] = T{ 0 }; this->data[0][2] = T{ 0 };
		this->data[1][0] = T{ 0 }; this->data[1][1] = T{ 1 }; this->data[1][2] = T{ 0 };
		this->data[2][0] = T{ 0 }; this->data[2][1] = T{ 0 }; this->data[2][2] = T{ 1 };
	}

	Vector4Type<T> GetVector4(int RowIndex)
	{
		return Vector4Type<T>(this->data[RowIndex][0], this->data[RowIndex][1], this->data[RowIndex][2], this->data[RowIndex][3]);
	}
};

template<typename T>
struct Matrix4x4 : Matrix<T, 4, 4>
{
	Matrix4x4(){ IdentityMatrix(); }

	void IdentityMatrix()
	{
		this->data[0][0] = T{ 1 }; this->data[0][1] = T{ 0 }; this->data[0][2] = T{ 0 }; this->data[0][3] = T{ 0 };
		this->data[1][0] = T{ 0 }; this->data[1][1] = T{ 1 }; this->data[1][2] = T{ 0 }; this->data[1][3] = T{ 0 };
		this->data[2][0] = T{ 0 }; this->data[2][1] = T{ 0 }; this->data[2][2] = T{ 1 }; this->data[2][3] = T{ 0 };
		this->data[3][0] = T{ 0 }; this->data[3][1] = T{ 0 }; this->data[3][2] = T{ 0 }; this->data[3][3] = T{ 1 };
	}

	Vector4Type<T> GetVector4(int RowIndex)
	{
		return Vector4Type<T>(this->data[RowIndex][0], this->data[RowIndex][1], this->data[RowIndex][2], this->data[RowIndex][3]);
	}
};

using Matrix3x3f = Matrix3x3<float>;
using Matrix4x4f = Matrix4x4<float>;