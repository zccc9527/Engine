#pragma once
#include <stdint.h>

//二维向量模板类
template<typename T>
struct Vector2Type
{
	T x;
	T y;

	Vector2Type(const T& x, const T& y) : x(x), y(y) {}
	Vector2Type(const T& v) : Vector2Type(v, v) {}
	Vector2Type() : Vector2Type(T{}) {}
	

};

//三维向量模板类
template<typename T>
struct Vector3Type : Vector2Type<T>
{
	T z;

	Vector3Type(const T& x, const T& y, const T& z) : Vector2Type<T>(x, y), z(z) {}
	Vector3Type(const T& v) : Vector3Type(v, v, v) {}
	Vector3Type() : Vector3Type(T{}){}
};

//四维向量模板类，通常用于矩阵计算，rgba
template<typename T>
struct Vector4Type : Vector3Type<T>
{
	T w;

	Vector4Type(const T& x, const T& y, const T& z, const T& w) : Vector3Type<T>(x, y, z), w(w) {}
	Vector4Type(const T& x, const T& y, const T& z) : Vector3Type<T>(x, y, z), w(1) {}
	Vector4Type(const T& v) : Vector4Type(v, v, v) {}
	Vector4Type() : Vector4Type(T{}) {}
};

//方便使用的重命名
using Vector2f = Vector2Type<float>;
using Vector2d = Vector2Type<double>;
using Vector2 = Vector2Type<int>;
using Vector3f = Vector3Type<float>;
using Vector3d = Vector3Type<double>;
using Vector3 = Vector3Type<int>;
using Vector4f = Vector4Type<float>;
using Vector4d = Vector4Type<double>;
using Vector4 = Vector4Type<int>;

using R8G8B8 = Vector3Type<uint8_t>;
using R8G8B8A8 = Vector4Type<uint8_t>;
using Quaternion = Vector4Type<float>;
using Color = Vector4Type<float>;
