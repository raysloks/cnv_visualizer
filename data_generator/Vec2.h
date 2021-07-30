#pragma once

class Vec3;

class Vec2
{
public:
	float x, y;

	Vec2(void);
	Vec2(float xy);
	Vec2(float x, float y);
	Vec2(const Vec3 & xy);
	
	float Len(void) const;
	float LenPwr(void) const;

	Vec2 & Normalize(void);
	Vec2 Normalized(void) const;

	float Dot(const Vec2 & rhs) const;

	Vec2 & Truncate(float lim);
	Vec2 Truncated(float lim) const;

	Vec2 & Rotate(float degs);
	
	const Vec2 & operator+=(const Vec2 & rhs);
	const Vec2 & operator-=(const Vec2 & rhs);
	const Vec2 & operator*=(const Vec2 & rhs);
	const Vec2 & operator/=(const Vec2 & rhs);
	const Vec2 & operator*=(float rhs);
	const Vec2 & operator/=(float rhs);
	Vec2 operator+(const Vec2 & rhs) const;
	Vec2 operator-(const Vec2 & rhs) const;
	Vec2 operator*(const Vec2 & rhs) const;
	Vec2 operator/(const Vec2 & rhs) const;
	Vec2 operator*(float rhs) const;
	Vec2 operator/(float rhs) const;

	bool operator!=(const Vec2 & rhs) const;
	bool operator==(const Vec2 & rhs) const;
	bool operator<(const Vec2 & rhs) const;
	bool operator<=(const Vec2 & rhs) const;
	bool operator>(const Vec2 & rhs) const;
	bool operator>=(const Vec2 & rhs) const;
};

Vec2 operator-(const Vec2 & rhs);