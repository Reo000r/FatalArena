#include "PlayerReinforcementManager.h"

#include <cassert>

StatsData PlayerReinforcementManager::_stats;

void PlayerReinforcementManager::SetStatsData(StatsData data)
{
	_stats = data;
}

StatsData PlayerReinforcementManager::GetStatsData()
{
	return _stats;
}

void PlayerReinforcementManager::AttachReinforcementData(ReinforcementData data)
{
	switch (data.type) {
	case ReinforcementType::MaxHealth:
		_stats.maxHealthMag += data.reinforceAmount;
		break;
	case ReinforcementType::Stamina:
		_stats.maxStaminaMag += data.reinforceAmount;
		break;
	case ReinforcementType::Strength:
		_stats.maxStrengthMag += data.reinforceAmount;
		break;

	default:
		assert(false && "不明なタイプ");
		break;
	}
}
