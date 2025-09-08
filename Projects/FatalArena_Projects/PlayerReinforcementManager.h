#pragma once

enum class ReinforcementType {
	MaxHealth,
	Stamina,
	Strength,
	TypeNum,
	None,
};

struct ReinforcementData {
	ReinforcementType type = ReinforcementType::None;
	float reinforceAmount = 0.0f;
};

struct StatsData {
	float maxHealth = 0.0f;
	float maxHealthMag = 1.0f;
	float maxStamina = 0.0f;
	float maxStaminaMag = 1.0f;
	float maxStrength = 0.0f;
	float maxStrengthMag = 1.0f;
};

/// <summary>
/// プレイヤーの強化管理を行う
/// ・デフォルトの値と強化値を保存し
/// ・強化教科内容の反映
/// </summary>
class PlayerReinforcementManager
{
public:

	/// <summary>
	/// ステータスデータを置き換える
	/// </summary>
	/// <param name="data"></param>
	static void SetStatsData(StatsData data);
	/// <summary>
	/// ステータスデータを取得する
	/// </summary>
	/// <returns></returns>
	static StatsData GetStatsData();

	/// <summary>
	/// 強化データを適用する
	/// </summary>
	/// <param name="data"></param>
	static void AttachReinforcementData(ReinforcementData data);

private:

	static StatsData _stats;

};

