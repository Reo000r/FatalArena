#include "ColliderDataInvertedCylinder.h"

ColliderDataInvertedCylinder::ColliderDataInvertedCylinder(
	bool isTrigger, bool isCollision, 
	Vector3 startToEnd, float innerRad, float outerRad) :
	ColliderData(PhysicsData::ColliderKind::InvertedCylinder, 
		isTrigger, isCollision),
	_innerRadius(innerRad),
	_outerRadius(outerRad),
	_startToEnd(startToEnd)
{
	// 反転していたなら代入しなおす
	if (innerRad > outerRad) {
		_innerRadius = outerRad;
		_outerRadius = innerRad;
	}
}
