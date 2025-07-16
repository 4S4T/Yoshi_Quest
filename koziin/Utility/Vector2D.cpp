#include "Vector2D.h"
#include <math.h>

const Vector2D Vector2D::operator/(const float& scalar) const {
	// 0œZ‚Ìl—¶
	if (fabsf(scalar) < 1.0e-6f) {
		return Vector2D(0.0f);
	}

	return Vector2D(this->x / scalar, this->y / scalar);
}

const Vector2D Vector2D::operator/(const Vector2D& v) const {
	// 0œZ‚Ìl—¶
	if (fabsf(v.x) < 1.0e-6f || fabsf(v.y) < 1.0e-6f) {
		return Vector2D(0.0f);
	}

	return Vector2D(this->x / v.x, this->y / v.y);
}

Vector2D& Vector2D::operator/=(const float& scalar) {
	// 0œZ‚Ìl—¶
	if (fabsf(scalar) < 1.0e-6f) {
		this->x = 0.0f;
		this->y = 0.0f;
	}
	else {
		this->x /= scalar;
		this->y /= scalar;
	}

	return *this;
}

Vector2D& Vector2D::operator/=(const Vector2D& v) {
	// 0œZ‚Ìl—¶
	if (fabsf(v.x) < 1.0e-6f || fabsf(v.y) < 1.0e-6f) {
		this->x = 0.0f;
		this->y = 0.0f;
	}
	else {
		this->x /= v.x;
		this->y /= v.y;
	}

	return *this;
}

float Vector2D::SqrLength() const {
	return Vector2D::Dot(*this);
}

float Vector2D::Length() const {
	return sqrtf(SqrLength());
}

Vector2D Vector2D::Normalize() const {
	return *this / Length();
}

float Vector2D::Dot(const Vector2D& a) {
	return ((a.x * a.x) + (a.y * a.y));
}

float Vector2D::Dot(const Vector2D& a, const Vector2D& b) {
	return ((a.x * b.x) + (a.y * b.y));
}

float Vector2D::Cross(const Vector2D& a, const Vector2D& b) {
	return (a.x * b.y - a.y * b.x);
}

Vector2D Vector2D::Lerp(const Vector2D& a, const Vector2D& b, const float t) {
	return a + ((b - a) * t);
}

float Vector2D::Distance(const Vector2D& a, const Vector2D& b) {
	return Vector2D(a - b).SqrLength();
}

// “™’l”äŠr
bool Vector2D::operator==(const Vector2D& v) const {
	return (fabsf(this->x - v.x) < 1.0e-6f) && (fabsf(this->y - v.y) < 1.0e-6f);
}

// ”ñ“™’l”äŠr
bool Vector2D::operator!=(const Vector2D& v) const {
	return !(*this == v);
}

float Vector2D::DistanceTo(const Vector2D& other) const {
	float dx = other.x - this->x;
	float dy = other.y - this->y;
	return sqrt(dx * dx + dy * dy);
}
