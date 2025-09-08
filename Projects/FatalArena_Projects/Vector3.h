#pragma once

// 暗黙的型変換のためのプロトタイプ宣言
namespace DxLib {
	struct tagVECTOR;
}

class Vector2;
class Vector3;
typedef Vector3 Position3;

class Vector3 final {
public:
	float x, y, z;

	Vector3() : x(0.0f), y(0.0f), z(0.0f) {};
	Vector3(float x_, float y_, float z_) :
		x(x_), y(y_), z(z_) // (_x等だとメンバの命名と被るため)
	{
	};

	// VECTORからVector3、Vector3からVECTORへ
	// 暗黙的に型変換するためのもの
	operator DxLib::tagVECTOR();
	operator DxLib::tagVECTOR() const;
	Vector3(const DxLib::tagVECTOR vector);

	// オペレータオーバーロード
	// (除算はassert使用)
	Vector3 operator+(const Vector3& v) const;
	Vector3 operator-(const Vector3& v) const;
	Vector3 operator*(const float& m) const;
	Vector3 operator/(const float& d) const;
	void operator+=(const Vector3& v);
	void operator-=(const Vector3& v);
	void operator*=(const float& m);
	void operator/=(const float& d);

	void operator=(const Vector2& vec2);
	void operator+=(const Vector2& vec2);
	Vector3 operator+(const Vector2& vec2) const;

	/// <summary>
	/// 符号反転
	/// </summary>
	/// <returns></returns>
	Vector3 operator-() const;

	// 比較演算
	bool operator==(const Vector3& v) const;
	bool operator!=(const Vector3& v) const;

	/// <summary>
	/// ベクトルの長さを返す
	/// </summary>
	/// <returns></returns>
	float Magnitude() const;
	/// <summary>
	/// ベクトルの長さを返す
	/// </summary>
	/// <returns></returns>
	float Magnitude(const Vector3& v) const;
	/// <summary>
	/// ベクトルの長さを返す
	/// </summary>
	/// <returns></returns>
	float Magnitude(const float& x_, const float& y_, const float& z_) const;

	/// <summary>
	/// ベクトルの二乗の長さを返す
	/// </summary>
	/// <returns></returns>
	float SqrMagnitude() const;
	/// <summary>
	/// ベクトルの二乗の長さを返す
	/// </summary>
	/// <returns></returns>
	float SqrMagnitude(const Vector3& v) const;
	/// <summary>
	/// ベクトルの二乗の長さを返す
	/// </summary>
	/// <returns></returns>
	float SqrMagnitude(const float& x_, const float& y_, const float& z_) const;

	/// <summary>
	/// aとbの間の距離を返す
	/// (Distance(a, b) は (a-b).magnitudeと同じ)
	/// </summary>
	/// <returns></returns>
	float Distance(const Vector3& a, const Vector3& b) const;

	/// <summary>
	/// 正規化する
	/// </summary>
	void Normalized();
	/// <summary>
	/// 正規化したベクトルを返す
	/// </summary>
	/// <returns></returns>
	Vector3 Normalize() const;

	/// <summary>
	/// 
	/// </summary>
	/// <param name="v"></param>
	/// <param name="scale"></param>
	/// <returns></returns>
	void VecScale(const float& scale);
};

/// <summary>
/// aからb へ tの割合だけ近づいた点を返す
/// tは 0-1 の範囲
/// tが 0   の場合、a を返す
/// tが 1   の場合、b を返す
/// tが 0.5 の場合、a と b の中点が返される
/// tが 0以下 の場合 a を返す(t=0と同じ)
/// tが 1以上 の場合 b を返す(t=1と同じ)
/// </summary>
/// <param name="a">始点</param>
/// <param name="b">終点</param>
/// <param name="t">範囲</param>
/// <returns></returns>
Vector3 Lerp(const Vector3& va, const Vector3& vb, const float& t);

/// <summary>
/// スケーリングしたベクトルを返す
/// </summary>
/// <param name="v"></param>
/// <param name="scale"></param>
/// <returns></returns>
Vector3 VecScale(const Vector3& v, const float& scale);

/// <summary>
/// 内積を返す
/// </summary>
/// <param name="va"></param>
/// <param name="vb"></param>
/// <returns></returns>
float Dot(const Vector3& va, const Vector3& vb);

/// <summary>
/// 外積を返す
/// (恐らく不完全)
/// </summary>
/// <param name="va"></param>
/// <param name="vb"></param>
/// <returns></returns>
//float Cross(const Vector3& va, const Vector3& vb);

/// <summary>
/// 外積を返す
/// </summary>
/// <param name="va"></param>
/// <param name="vb"></param>
/// <returns></returns>
Vector3 Cross(const Vector3& va, const Vector3& vb);

/// <summary>
/// 内積
/// </summary>
/// <param name="va"></param>
/// <param name="vb"></param>
/// <returns></returns>
float operator*(const Vector3& va, const Vector3& vb);

/// <summary>
/// 外積
/// </summary>
/// <param name="va"></param>
/// <param name="vb"></param>
/// <returns></returns>
Vector3 operator%(const Vector3& va, const Vector3& vb);

/// <summary>
/// 反射ベクトルを求める
/// </summary>
/// <param name="v">入射ベクトル</param>
/// <param name="normal">法線ベクトル</param>
/// <returns></returns>
Vector3 Reflect(const Vector3 v, Vector3 normal);


Vector3 Vector3Right();
Vector3 Vector3Left();
Vector3 Vector3Up();
Vector3 Vector3Down();
Vector3 Vector3Front();
Vector3 Vector3Back();
