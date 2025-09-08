#pragma once
#include "ItemBase.h"

class ItemStrength : public ItemBase
{
public:
	ItemStrength(BuffData data, int modelHandle,
		std::weak_ptr<PlayerBuffManager> manager);
	~ItemStrength();

	/// <summary>
	/// 初期化
	/// </summary>
	void Init(Vector3 modelTransOffset);

private:

	void PlayGetSE() override;
};

