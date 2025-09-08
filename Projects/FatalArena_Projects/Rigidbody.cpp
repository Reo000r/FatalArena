#include "Rigidbody.h"

Rigidbody::Rigidbody() :
	pos(),
	dir(),
	vel(),
	useGravity(false)
{
}

void Rigidbody::Init(bool useGravity_)
{
	pos = {};
	dir = {};
	vel = {};
	useGravity = useGravity_;
}

void Rigidbody::SetVel(const Vector3& set)
{
	vel = set;
	// 長さがあるなら
	if (vel.SqrMagnitude() > 0)
	{
		// 正規化したベクトルを向きとして代入
		dir = vel.Normalize();
	}
}
