#pragma once

class Vector2;
typedef Vector2 Position2;
class Vector3;

class Vector2 final {
public:
	float x, y;

public:
	Vector2() : x(0.0f), y(0.0f)
	{
	}

	Vector2(float _x, float _y) : x(_x), y(_y)
	{
	}

	void operator+=(const Vector2& vec);
	void operator-=(const Vector2& vec);
	void operator*=(float scale);
	void operator/=(float scale);
	Vector2 operator+(const Vector2& vec) const;
	Vector2 operator-(const Vector2& vec) const;
	Vector2 operator*(float scale) const;
	Vector2 operator/(float scale) const;

	void operator=(const Vector3& vec3);
	void operator+=(const Vector3& vec3);
	void operator-=(const Vector3& vec3);
	Vector3 operator+(const Vector3& vec3) const;

	/// <summary>
	/// 符号反転
	/// </summary>
	/// <returns></returns>
	Vector2 operator-() const;

	// 便利な関数群

	/// <summary>
	/// このベクトルの2乗の長さを返す
	/// </summary>
	/// <returns></returns>
	float SqrMagnitude() const;

	/// <summary>
	/// ベクトルの2乗の長さを返す
	/// </summary>
	/// <param name="a"></param>
	/// <returns></returns>
	float SqrMagnitude(const Vector2& a)const;

	/// <summary>
	/// ベクトルの2乗の長さを返す
	/// </summary>
	/// <param name="_x"></param>
	/// <param name="_y"></param>
	/// <returns></returns>
	float SqrMagnitude(float _x, float _y)const;

	/// <summary>
	/// このベクトルの長さを返す
	/// </summary>
	/// <returns></returns>
	float Magnitude() const;

	/// <summary>
	/// ベクトルの長さを返す
	/// </summary>
	/// <param name="a"></param>
	/// <returns></returns>
	float Magnitude(const Vector2& a) const;

	/// <summary>
	/// ベクトルの長さを返す
	/// </summary>
	/// <param name="_x"></param>
	/// <param name="_y"></param>
	/// <returns></returns>
	float Magnitude(float _x, float _y) const;

	/// <summary>
	/// aとbの間の距離を返す
	/// (Distance(a, b) は (a-b).magnitudeと同じ)
	/// </summary>
	/// <param name="a"></param>
	/// <param name="b"></param>
	/// <returns></returns>
	float Distance(const Vector2& a, const Vector2& b) const;

	/// <summary>
	/// 正規化(normalized)されたとき、方向を維持したままで
	/// 長さが 1.0 のベクトルを返す
	/// </summary>
	/// <param name="a"></param>
	/// <returns></returns>
	void Normalized();

	/// <summary>
	/// 正規化(normalized)されたとき、方向を維持したままで
	/// 長さが 1.0 のベクトルを返す
	/// </summary>
	/// <param name="a"></param>
	/// <returns></returns>
	Vector2 Normalize() const;

	/// <summary>
	/// aからb へ tの割合だけ近づいた点を返す
	/// tは 0-1 の範囲
	/// tが 0   の場合、a を返す
	/// tが 1   の場合、b を返す
	/// tが 0.5 の場合、a と b の中点が返される
	/// tが 0以下 の場合 a を返す(t=0と同じ)
	/// tが 1以上 の場合 b を返す(t=1と同じ)
	/// </summary>
	/// <param name="a"></param>
	/// <param name="b"></param>
	/// <param name="t"></param>
	/// <returns></returns>
	Vector2 Lerp(const Vector2& a, const Vector2& b, const float& t) const;
};