#include "Vec2.h"

#include "Vec3.h"

#define _USE_MATH_DEFINES

#include <cmath>

Vec2::Vec2(void) : x(0.0f), y(0.0f)
{
}

Vec2::Vec2(float xy) : x(xy), y(xy)
{
}

Vec2::Vec2(float x, float y) : x(x), y(y)
{
}

Vec2::Vec2(const Vec3 & xy) : x(xy.x), y(xy.y)
{
}

float Vec2::Len(void) const
{
	return sqrtf(x * x + y * y);
}

float Vec2::LenPwr(void) const
{
	return x * x + y * y;
}

Vec2& Vec2::Normalize(void)
{
	float l = Len();
	if (l != 0.0f)
		*this /= l;
	return *this;
}

Vec2 Vec2::Normalized(void) const
{
	float l = Len();
	if (l != 0.0f)
		return *this / l;
	return *this;
}

float Vec2::Dot(const Vec2& rhs) const
{
	return x * rhs.x + y * rhs.y;
}

Vec2& Vec2::Truncate(float lim)
{
	float l = LenPwr();
	if (l > lim * lim)
		*this *= lim / sqrtf(l);
	return *this;
}

Vec2 Vec2::Truncated(float lim) const
{
	float l = LenPwr();
	if (l > lim * lim)
		return Vec2(*this) * lim / sqrtf(l);
	return Vec2(*this);
}

Vec2 & Vec2::Rotate(float degs)
{
	float rads = degs * M_PI / 180.0f;
	float sin = sinf(rads);
	float cos = cosf(rads);
	*this = Vec2(x * cos + y * sin, y * cos - x * sin);
	return *this;
}

const Vec2& Vec2::operator+=(const Vec2& rhs)
{
	x += rhs.x;
	y += rhs.y;
	return *this;
}

const Vec2& Vec2::operator-=(const Vec2& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}

const Vec2& Vec2::operator*=(const Vec2& rhs)
{
	x *= rhs.x;
	y *= rhs.y;
	return *this;
}

const Vec2 & Vec2::operator/=(const Vec2 & rhs)
{
	x /= rhs.x;
	y /= rhs.y;
	return *this;
}

const Vec2& Vec2::operator*=(float rhs)
{
	x *= rhs;
	y *= rhs;
	return *this;
}

const Vec2& Vec2::operator/=(float rhs)
{
	x /= rhs;
	y /= rhs;
	return *this;
}

Vec2 Vec2::operator+(const Vec2& rhs) const
{
	return Vec2(x + rhs.x, y + rhs.y);
}

Vec2 Vec2::operator-(const Vec2& rhs) const
{
	return Vec2(x - rhs.x, y - rhs.y);
}

Vec2 Vec2::operator*(const Vec2& rhs) const
{
	return Vec2(x * rhs.x, y * rhs.y);
}

Vec2 Vec2::operator/(const Vec2 & rhs) const
{
	return Vec2(x / rhs.x, y / rhs.y);
}

Vec2 Vec2::operator*(float rhs) const
{
	return Vec2(x * rhs, y * rhs);
}

Vec2 Vec2::operator/(float rhs) const
{
	return Vec2(x / rhs, y / rhs);
}

bool Vec2::operator!=(const Vec2& rhs) const
{
	return x != rhs.x || y != rhs.y;
}

bool Vec2::operator==(const Vec2& rhs) const
{
	return x == rhs.x && y == rhs.y;
}

bool Vec2::operator<(const Vec2& rhs) const
{
	return x < rhs.x && y < rhs.y;
}

bool Vec2::operator<=(const Vec2& rhs) const
{
	return x <= rhs.x && y <= rhs.y;
}

bool Vec2::operator>(const Vec2& rhs) const
{
	return x > rhs.x && y > rhs.y;
}

bool Vec2::operator>=(const Vec2& rhs) const
{
	return x >= rhs.x && y >= rhs.y;
}

Vec2 operator-(const Vec2& rhs)
{
	return Vec2(-rhs.x, -rhs.y);
}