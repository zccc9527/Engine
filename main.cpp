#include <iostream>
#include "Engine/Math/vector.h"
#include "Engine/Math/matrix.h"

int main()
{
	Vector2Type<float> a(10, 20);
	Vector3Type<float> b(1.f, 2.f, 3.f);
	Vector4Type<int> c(1.1, 2.2, 3.3);
	/*a.x = 10;
	a.y = 20;*/
	std::cout << a.x << "  " << a.y << std::endl;
	std::cout << b.x << "  " << b.y << "  " << b.z << std::endl;
	std::cout << c.x << "  " << c.y << "  " << c.z << "  " << c.w << std::endl;

	Matrix4x4f d;
	std:: cout << d.data[1][2] << std::endl;

	system("pause");
	return 0;
}