#pragma once
#include "ColliderData.h"

class Collider;
class Physics;

/// <summary>
/// 反転した円柱
/// 内側ではなく外側に当たり判定がある
/// 向きはy+方向固定
/// </summary>
class ColliderDataInvertedCylinder : public ColliderData {
public:
	ColliderDataInvertedCylinder(bool isTrigger, bool isCollision, 
		Vector3 startToEnd, float innerRad, float outerRad);

	/// <summary>
	/// 内側の半径を取得
	/// </summary>
	/// <returns>内側の半径</returns>
	float GetInnerRadius() const { return _innerRadius; }

	/// <summary>
	/// 外側の半径を取得
	/// </summary>
	/// <returns>外側の半径</returns>
	float GetOuterRadius() const { return _outerRadius; }

	/// <summary>
	/// 当たり判定の厚みを取得
	/// </summary>
	/// <returns>厚み</returns>
	float GetThickness() const { return _outerRadius - _innerRadius; }

	/// <summary>
	/// 高さを取得
	/// </summary>
	/// <returns>高さ</returns>

	// 以降の変数のアクセス権を渡す
	friend Collider;
	friend Physics;

private:
	// 高さ
	Vector3 _startToEnd;

	// 半径
	float _innerRadius;
	float _outerRadius;

};

