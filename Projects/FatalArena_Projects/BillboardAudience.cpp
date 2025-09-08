#include "BillboardAudience.h"

#include <cmath>
#include <DxLib.h>

namespace {
	constexpr float kScaleMul = 500.0f;

	constexpr float kAnimationLoopFrame = 10.0f;
	constexpr float kAnimationMoveMul = 3.0f;
}

BillboardAudience::BillboardAudience(int modelHandle, float defaultProgress) :
	_modelHandle(modelHandle),
	_pos(),
	_animFrame((int)((kAnimationLoopFrame * kAnimationLoopFrame) * defaultProgress))
{
}

BillboardAudience::~BillboardAudience()
{
	if (_modelHandle != -1) {
		DeleteGraph(_modelHandle);
	}
}

void BillboardAudience::SetPos(Position3 pos)
{
	_pos = pos;
}

void BillboardAudience::Update()
{
	// 上下に動かす
	_animFrame++;
	_pos.y += sinf(_animFrame / kAnimationLoopFrame) * kAnimationMoveMul;
}

void BillboardAudience::Draw()
{
	DrawBillboard3D(
		_pos,			// 描画位置
		0.5f, 0.0f,		// 画像の中心位置
		kScaleMul, 0.0f,// 拡大倍率、回転量
		_modelHandle,	// 画像ハンドル
		true);			// 透過フラグ
}
