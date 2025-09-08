#include "ItemScoreBoost.h"
#include "SoundManager.h"

ItemScoreBoost::ItemScoreBoost(BuffData data, int modelHandle,
	std::weak_ptr<PlayerBuffManager> manager) :
	ItemBase(data, modelHandle, manager)
{
	// 処理なし
}

ItemScoreBoost::~ItemScoreBoost()
{
	// 処理なし
}

void ItemScoreBoost::Init(Vector3 modelTransOffset)
{
	_modelOffset = modelTransOffset;
}

void ItemScoreBoost::PlayGetSE()
{
	SoundManager::GetInstance().PlaySoundType(SEType::ItemScoreBoost);
}
