#include "PlayerBuffManager.h"
#include "Player.h"
#include "PlayerBuffGaugeDrawer.h"

#include <cassert>

void PlayerBuffManager::Init(std::weak_ptr<Player> owner)
{
	_owner = std::static_pointer_cast<Player>(owner.lock());
	InitBuff(); 
	_gaugeDrawer = std::make_shared<PlayerBuffGaugeDrawer>();
	_gaugeDrawer->Init(shared_from_this());
}

void PlayerBuffManager::Update()
{
	UpdateBuff();
}

void PlayerBuffManager::Draw()
{
	_gaugeDrawer->Draw();
}

void PlayerBuffManager::AttachBuff(BuffData input)
{
	// 更新対象のアイテムをvectorの末尾に置いてから削除する
	_buffs.erase(
		std::remove_if(_buffs.begin(), _buffs.end(),
			[input](const BuffData& buff) {
				return (buff.type == input.type);
			}),
		_buffs.end()
	);
	// データを登録する
	_buffs.emplace_back(input);

	// 1f多く回復
	if (input.type == BuffType::Heal) {
		_owner.lock()->Heal(input.amount);
	}
}

BuffData PlayerBuffManager::GetData(BuffType buff) const
{
	for (const auto data : _buffs) {
		if (data.type == buff) {
			return data;
		}
	}
	assert(false && "未登録のバフタイプが選択された");
	return BuffData();
}

void PlayerBuffManager::InitBuff()
{
	_buffs.clear();
	// 効果を追加
	BuffData data = {};
	data.type = BuffType::Heal;
	_buffs.emplace_back(data);
	data.type = BuffType::ScoreBoost;
	_buffs.emplace_back(data);
	data.type = BuffType::Strength;
	_buffs.emplace_back(data);
}

void PlayerBuffManager::UpdateBuff()
{
	for (auto& data : _buffs) {
		if (!data.isActive) continue;
		
		data.activeFrame--;			// 残り時間を減らす
		// 残り時間がなければ飛ばす
		if (data.activeFrame <= 0 && data.isActive) {
			data.amount = 0.0f;
			data.isActive = false;
			continue;
		}

		switch (data.type) {
		case BuffType::Heal:		// 継続回復処理
			_owner.lock()->Heal(data.amount);
			break;
		case BuffType::ScoreBoost:	// スコア増加処理
			break;
		case BuffType::Strength:	// 攻撃力増加処理
			break;
		}
	}
}
