#pragma once

// 2次元ベクトルクラス
class Vector2D {
public:
	float x;
	float y;

public:
	Vector2D() : x(0.0f), y(0.0f) {}
	Vector2D(float scalar) : x(scalar), y(scalar) {}
	Vector2D(float mx, float my) : x(mx), y(my) {}
	~Vector2D() {}

public:
	// 代入
	Vector2D& operator=(const Vector2D& v) {
		this->x = v.x;
		this->y = v.y;

		return *this;
	}

	// 足し算
	const Vector2D operator+(const Vector2D& v) const {
		return Vector2D(this->x + v.x, this->y + v.y);
	}
	Vector2D& operator+=(const Vector2D& v) {
		this->x += v.x;
		this->y += v.y;

		return *this;
	}

	// 引き算
	const Vector2D operator-(const Vector2D& v) const {
		return Vector2D(this->x - v.x, this->y - v.y);
	}
	Vector2D& operator-=(const Vector2D& v) {
		this->x -= v.x;
		this->y -= v.y;

		return *this;
	}

	// 掛け算
	const Vector2D operator*(const float& scalar) const {
		return Vector2D(this->x * scalar, this->y * scalar);
	}
	const Vector2D operator*(const Vector2D& v) const {
		return Vector2D(this->x * v.x, this->y * v.y);
	}
	Vector2D& operator*=(const float& scalar) {
		this->x *= scalar;
		this->y *= scalar;

		return *this;
	}
	Vector2D& operator*=(const Vector2D& v) {
		this->x *= v.x;
		this->y *= v.y;

		return *this;
	}

	// 割り算
	const Vector2D operator/(const float& scalar) const;
	const Vector2D operator/(const Vector2D& v) const;
	Vector2D& operator/=(const float& scalar);
	Vector2D& operator/=(const Vector2D& v);


	// **等値・非等値比較の宣言のみ**
	bool operator==(const Vector2D& v) const;
	bool operator!=(const Vector2D& v) const;

public:
	// ベクトルの長さ
	float SqrLength() const;
	float Length() const;

	// 正規化
	Vector2D Normalize() const;

	// 内積
	static float Dot(const Vector2D& a);
	static float Dot(const Vector2D& a, const Vector2D& b);
	// 外積
	static float Cross(const Vector2D& a, const Vector2D& b);
	// 線形補間
	static Vector2D Lerp(const Vector2D& a, const Vector2D& b, const float t);

	// 2点間の距離
	static float Distance(const Vector2D& a, const Vector2D& b);
};