#include "ReinforcementCard.h"

#include <DxLib.h>
#include <cassert>

namespace {
	constexpr int kDrawIconOffset = -64;
}

ReinforcementCard::ReinforcementCard() :
	_cardHandle(-1),
	_iconHandle(-1),
	_centerPos(),
	_cardScaleOffset(1.0f),
	_iconScaleOffset(1.0f),
	_cardWidth(0),
	_cardHeight(0),
	_iconWidth(0),
	_iconHeight(0)
{
	// 処理なし
}

ReinforcementCard::~ReinforcementCard()
{
	if (_cardHandle != -1) {
		DeleteGraph(_cardHandle);
	}
	if (_iconHandle != -1) {
		DeleteGraph(_iconHandle);
	}
	
}

void ReinforcementCard::Init(int cardHandle, int iconHandle, Position3 centerPos, 
	float cardScaleOffset, float iconScaleOffset)
{
	_cardHandle = cardHandle;
	_iconHandle = iconHandle;
	_centerPos = centerPos;
	_cardScaleOffset = cardScaleOffset;
	_iconScaleOffset = iconScaleOffset;

	assert(_cardHandle > 0 && "不正なカード画像");
	assert(_iconHandle > 0 && "不正なアイコン画像");

	// 画像のサイズを取得
	GetGraphSize(_cardHandle, &_cardWidth, &_cardHeight);
	GetGraphSize(_iconHandle, &_iconWidth, &_iconHeight);
}

void ReinforcementCard::Update()
{
	// 処理なし
}

void ReinforcementCard::Draw()
{
	// アイコン位置補正
	Position3 iconPos = _centerPos;
	iconPos.y += kDrawIconOffset * _cardScaleOffset;

	// カード、アイコンの描画
	DrawRectRotaGraph(
		(int)_centerPos.x, (int)_centerPos.y,	// 描画中心座標
		0, 0,						// 切り取り開始位置
		_cardWidth, _cardHeight,	// 切り取り終了位置
		_cardScaleOffset, 0.0,		// 拡縮、角度補正
		_cardHandle,				// 画像ハンドル
		true,						// 透過情報
		false, false);				// xyの反転

	DrawRectRotaGraph(
		(int)iconPos.x, (int)iconPos.y,		// 描画中心座標
		0, 0,						// 切り取り開始位置
		_iconWidth, _iconHeight,	// 切り取り終了位置
		_iconScaleOffset, 0.0,		// 拡縮、角度補正
		_iconHandle,				// 画像ハンドル
		true,						// 透過情報
		false, false);				// xyの反転
}
