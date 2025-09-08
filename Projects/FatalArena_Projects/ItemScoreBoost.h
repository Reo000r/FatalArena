#pragma once
#include "ItemBase.h"

class ItemScoreBoost : public ItemBase
{
public:
	ItemScoreBoost(BuffData data, int modelHandle,
		std::weak_ptr<PlayerBuffManager> manager);
	~ItemScoreBoost();

	/// <summary>
	/// 初期化
	/// </summary>
	void Init(Vector3 modelTransOffset);

private:

	void PlayGetSE() override;
};

