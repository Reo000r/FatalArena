#include "DebugDraw.h"
#include <DxLib.h>

DebugDraw& DebugDraw::GetInstance()
{
	static DebugDraw ret;
	return ret;
}

void DebugDraw::Clear()
{
	_lineInfo.clear();
	_sphereInfo.clear();
	_capsuleInfo.clear();
}

void DebugDraw::Draw()
{
	// 線分描画
	for (const auto& item : _lineInfo)
	{
		DxLib::DrawLine3D(
			item.start,
			item.end,
			item.color);
	}
	// 球描画
	for (const auto& item : _sphereInfo)
	{
		DxLib::DrawSphere3D(
			item.center,
			item.rad,
			16,
			item.color,
			item.color,
			false);
	}
	// カプセル描画
	for (const auto& item : _capsuleInfo)
	{
		DxLib::DrawCapsule3D(
			item.start,
			item.end,
			item.rad,
			16,
			item.color,
			item.color,
			false);
	}
}

void DebugDraw::DrawLine(const Vector3& start, const Vector3& end, int color)
{
	LineInfo info;
	info.start = start;
	info.end = end;
	info.color = color;
	_lineInfo.emplace_back(info);
}

void DebugDraw::DrawSphere(const Vector3& center, float rad, int color)
{
	SphereInfo info;
	info.center = center;
	info.rad = rad;
	info.color = color;
	_sphereInfo.emplace_back(info);
}

void DebugDraw::DrawCapsule(const Vector3& start, const Vector3& end, float rad, int color)
{
	CapsuleInfo info;
	info.start = start;
	info.end = end;
	info.rad = rad;
	info.color = color;
	_capsuleInfo.emplace_back(info);
}

DebugDraw::DebugDraw() {
	Clear();
}
