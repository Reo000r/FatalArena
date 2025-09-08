#pragma once
#include "ItemBase.h"

class ItemHeal : public ItemBase
{
public:
	ItemHeal(BuffData data, int modelHandle, 
		std::weak_ptr<PlayerBuffManager> manager);
	~ItemHeal();

	/// <summary>
	/// 初期化
	/// </summary>
	void Init(Vector3 modelTransOffset);

private:

	void PlayGetSE() override;
};

