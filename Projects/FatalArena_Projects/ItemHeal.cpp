#include "ItemHeal.h"
#include "PlayerBuffManager.h"
#include "SoundManager.h"

ItemHeal::ItemHeal(BuffData data, int modelHandle,
	std::weak_ptr<PlayerBuffManager> manager) :
	ItemBase(data, modelHandle, manager)
{
	// 処理なし
}

ItemHeal::~ItemHeal()
{
	// 処理なし
}

void ItemHeal::Init(Vector3 modelTransOffset)
{
	_modelOffset = modelTransOffset;
}

void ItemHeal::PlayGetSE()
{
	SoundManager::GetInstance().PlaySoundType(SEType::ItemHeal);
}
