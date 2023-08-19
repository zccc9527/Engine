#pragma once
#include "vector.h"

//¾ØÕóÄ£°åÀà
template<typename T, int Rows, int Cols>
struct Matrix
{
	T data[Rows][Cols];

	Matrix()
	{
		for (int i = 0; i < Rows; i++)
		{
			for (int j = 0; j < Cols; j++)
			{
				data[i][j] = i == j ? 1 : 0;
			}
		}
	}
	/*operator T[][]()
	{
		return data;
	}*/
};

using Matrix4x4f = Matrix<float, 4, 4>;