#include "Vector3.h"
#include "Vector2.h"

#include <cmath>		// 各種計算用

#include <DxLib.h>


Vector3::operator DxLib::tagVECTOR() {
	return VGet(x, y, z);
}

Vector3::operator DxLib::tagVECTOR() const {
	return VGet(x, y, z);
}

Vector3::Vector3(const DxLib::tagVECTOR vector) :
	x(vector.x), y(vector.y), z(vector.z)
{
}

Vector3 Vector3::operator+(const Vector3& v) const {
	return Vector3(x + v.x, y + v.y, z + v.z);
}

Vector3 Vector3::operator-(const Vector3& v) const {
	return Vector3(x - v.x, y - v.y, z - v.z);
}

Vector3 Vector3::operator*(const float& m) const {
	return Vector3(x * m, y * m, z * m);
}

Vector3 Vector3::operator/(const float& d) const {
	if (d == 0.0f) {
#ifdef USE_ASSERT_GEOMETRY
		assert(false && "0除算");
#endif // USE_ASSERT_GEOMETRY
		return Vector3(0.0f, 0.0f, 0.0f);
	}
	return Vector3(x / d, y / d, z / d);
}

void Vector3::operator+=(const Vector3& v) {
	x += v.x;
	y += v.y;
	z += v.z;
}

void Vector3::operator-=(const Vector3& v) {
	x -= v.x;
	y -= v.y;
	z -= v.z;
}

void Vector3::operator*=(const float& m) {
	x *= m;
	y *= m;
	z *= m;
}

void Vector3::operator/=(const float& d) {
	if (d == 0.0f) {
#ifdef USE_ASSERT_GEOMETRY
		assert(false && "0除算");
#endif // USE_ASSERT_GEOMETRY
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		return;
	}
	x /= d;
	y /= d;
	z /= d;
}

void Vector3::operator=(const Vector2& vec2) {
	x = vec2.x;
	y = vec2.y;
	z = 0.0f;
}

void Vector3::operator+=(const Vector2& vec2) {
	x += vec2.x;
	y += vec2.y;
	z += 0.0f;
}

Vector3 Vector3::operator+(const Vector2& vec2) const {
	return Vector3(x + vec2.x, y + vec2.y, z);
}

Vector3 Vector3::operator-() const {
	return Vector3(-x, -y, -z);
}

bool Vector3::operator==(const Vector3& v) const
{
	return ((this->x == v.x) &&
			(this->y == v.y) &&
			(this->z == v.z));
}

bool Vector3::operator!=(const Vector3& v) const
{
	return !(*this == v);
}

float Vector3::Magnitude() const {
	return Magnitude(*this);
}

float Vector3::Magnitude(const Vector3& v) const {
	return Magnitude(v.x, v.y, v.z);
}

float Vector3::Magnitude(const float& x_, const float& y_, const float& z_) const {
	return sqrtf(SqrMagnitude(x_, y_, z_));
	// 分解↓
	//float temp = SqrMagnitude(Vector3(x_,y_,z_));
	//float abs = sqrtf(temp);
	//return abs;
}

float Vector3::SqrMagnitude() const {
	return SqrMagnitude(*this);
}

float Vector3::SqrMagnitude(const Vector3& v) const {
	return SqrMagnitude(v.x, v.y, v.z);
}

float Vector3::SqrMagnitude(const float& x_, const float& y_, const float& z_) const {
	return x_*x_ + y_*y_ + z_*z_;
}

float Vector3::Distance(const Vector3& a, const Vector3& b) const {
	return sqrtf(SqrMagnitude(a - b));
	// 分解↓
	//float fTemp = SqrMagnitude(Vector3(a - b));
	//float abs = sqrtf(fTemp);
	//return abs;
}

void Vector3::Normalized() {
	*this = Normalize();
}

Vector3 Vector3::Normalize() const {
	float abs = sqrtf(SqrMagnitude());
	if (abs == 0.0f) {
		//#ifdef USE_ASSERT_GEOMETRY
		//		assert(false && "0除算");
		//#endif // USE_ASSERT_GEOMETRY
		return *this;
	}
	return Vector3(*this / abs);
}

void Vector3::VecScale(const float& scale) {
	x *= scale;
	y *= scale;
	z *= scale;
}

Vector3 Lerp(const Vector3& va, const Vector3& vb, const float& t) {
	// 長さが0-1ではないなら相応しい値を返す
	if (t < 0.0f) return va;	// 短い場合
	if (t > 1.0f) return vb;	// 長い場合
	return va + (vb - va) * t;
}

Vector3 VecScale(const Vector3& v, const float& scale) {
	return Vector3(v.x * scale, v.y * scale, v.z * scale);
	//return Vector3(v.y * scale, v.z * scale, v.x * scale);
}

float Dot(const Vector3& va, const Vector3& vb) {
	return va.x * vb.x + va.y * vb.y + va.z * vb.z;
}

Vector3 Cross(const Vector3& va, const Vector3& vb) {
	Vector3 ret = Vector3(
		va.y * vb.z - va.z * vb.y,
		va.z * vb.x - va.x * vb.z,
		va.x * vb.y - va.y * vb.x);
	return ret;
}

float operator*(const Vector3& va, const Vector3& vb) {
	return Dot(va, vb);
}

Vector3 operator%(const Vector3& va, const Vector3& vb) {
	return Cross(va, vb);
}

Vector3 Reflect(const Vector3 v, Vector3 normal)
{
	// 正規化されていない場合に備えて正規化
	normal.Normalized();
	// 反射ベクトルを計算
	return v - normal * 2.0f * Dot(v, normal);
}

Vector3 Vector3Right()
{
	return Vector3(1.0f, 0.0f, 0.0f);
}

Vector3 Vector3Left()
{
	return Vector3(-1.0f, 0.0f, 0.0f);
}

Vector3 Vector3Up()
{
	return Vector3(0.0f, 1.0f, 0.0f);
}

Vector3 Vector3Down()
{
	return Vector3(0.0f, -1.0f, 0.0f);
}

Vector3 Vector3Front()
{
	return Vector3(0.0f, 0.0f, 1.0f);
}

Vector3 Vector3Back()
{
	return Vector3(0.0f, 0.0f, -1.0f);
}
