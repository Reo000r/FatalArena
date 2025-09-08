#pragma once

#include "Vector3.h"

/// <summary>
/// 物理、衝突判定に必要なデータの塊
/// </summary>
class Rigidbody
{
public:
	Rigidbody();
	void Init(bool useGravity_ = false);	// 初期化

	// getter
	const Position3& GetPos() const { return pos; }
	const Vector3& GetDir() const { return dir; }
	const Vector3& GetVel() const { return vel; }
	bool UseGravity() const { return useGravity; }
	// setter
	void SetPos(const Position3& set) { pos = set; }
	void SetVel(const Vector3& set);
	void SetUseGravity(bool set) { useGravity = set; }

private:
	Position3	pos;
	Vector3	dir;
	Vector3	vel;
	bool	useGravity;
};

