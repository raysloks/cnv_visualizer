#include "Vec3.h"

#include "Vec2.h"

#include <cmath>

Vec3::Vec3(void) : x(0.0f), y(0.0f), z(0.0f)
{
}

Vec3::Vec3(float xyz) : x(xyz), y(xyz), z(xyz)
{
}

Vec3::Vec3(float x, float y) : x(x), y(y), z(0.0f)
{
}

Vec3::Vec3(const Vec2 & xy) : x(xy.x), y(xy.y), z(0.0f)
{
}

Vec3::Vec3(float x, float y, float z) : x(x), y(y), z(z)
{
}

float Vec3::Len(void) const
{
	return sqrtf(x * x + y * y + z * z);
}

float Vec3::LenPwr(void) const
{
	return x * x + y * y + z * z;
}

Vec3& Vec3::Normalize(void)
{
	float l = Len();
	if (l != 0.0f)
		*this /= l;
	return *this;
}

Vec3 Vec3::Normalized(void) const
{
	float l = Len();
	if (l != 0.0f)
		return *this / l;
	return *this;
}

float Vec3::Dot(const Vec3& rhs) const
{
	return x * rhs.x + y * rhs.y + z * rhs.z;
}

Vec3 Vec3::Cross(const Vec3& rhs) const
{
	return Vec3(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
}

const Vec3& Vec3::operator+=(const Vec3& rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	return *this;
}

const Vec3& Vec3::operator-=(const Vec3& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	return *this;
}

const Vec3& Vec3::operator*=(const Vec3& rhs)
{
	x *= rhs.x;
	y *= rhs.y;
	z *= rhs.z;
	return *this;
}

const Vec3 & Vec3::operator/=(const Vec3 & rhs)
{
	x /= rhs.x;
	y /= rhs.y;
	z /= rhs.z;
	return *this;
}

const Vec3& Vec3::operator*=(float rhs)
{
	x *= rhs;
	y *= rhs;
	z *= rhs;
	return *this;
}

const Vec3& Vec3::operator/=(float rhs)
{
	x /= rhs;
	y /= rhs;
	z /= rhs;
	return *this;
}

Vec3 Vec3::operator+(const Vec3& rhs) const
{
	return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
}

Vec3 Vec3::operator-(const Vec3& rhs) const
{
	return Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
}

Vec3 Vec3::operator*(const Vec3& rhs) const
{
	return Vec3(x * rhs.x, y * rhs.y, z * rhs.z);
}

Vec3 Vec3::operator/(const Vec3 & rhs) const
{
	return Vec3(x / rhs.x, y / rhs.y, z / rhs.z);
}

Vec3 Vec3::operator*(float rhs) const
{
	return Vec3(x * rhs, y * rhs, z * rhs);
}

Vec3 Vec3::operator/(float rhs) const
{
	return Vec3(x / rhs, y / rhs, z / rhs);
}

bool Vec3::operator!=(const Vec3& rhs) const
{
	return x != rhs.x || y != rhs.y || z != rhs.z;
}

bool Vec3::operator==(const Vec3& rhs) const
{
	return x == rhs.x && y == rhs.y && z == rhs.z;
}

bool Vec3::operator<(const Vec3& rhs) const
{
	return x < rhs.x && y < rhs.y && z < rhs.z;
}

bool Vec3::operator<=(const Vec3& rhs) const
{
	return x <= rhs.x && y <= rhs.y && z <= rhs.z;
}

bool Vec3::operator>(const Vec3& rhs) const
{
	return x > rhs.x && y > rhs.y && z > rhs.z;
}

bool Vec3::operator>=(const Vec3& rhs) const
{
	return x >= rhs.x && y >= rhs.y && z >= rhs.z;
}

Vec3 operator-(const Vec3& rhs)
{
	return Vec3(-rhs.x, -rhs.y, -rhs.z);
}