#pragma once
#include <memory>
#include <unordered_map>

class Player;
class PlayerBuffGaugeDrawer;

enum class BuffType {
	Heal,		// 継続回復
	ScoreBoost,	// 取得スコア増加
	Strength,	// 攻撃力増加
	TypeNum,	// タイプの数
	None,
};

struct BuffData {
	BuffType type = BuffType::None;
	int activeFrame = 0;
	int maxActiveFrame = 0;
	float amount = 0.0f;
	bool isActive = false;
};

/// <summary>
/// Playerのバフ状態を管理する
/// </summary>
class PlayerBuffManager final : public std::enable_shared_from_this<PlayerBuffManager>
{
public:

	void Init(std::weak_ptr<Player> owner);
	void Update();
	void Draw();

	/// <summary>
	/// 指定されたタイプのバフデータを更新する
	/// </summary>
	void AttachBuff(BuffData input);
	/// <summary>
	/// バフデータを返す
	/// </summary>
	BuffData GetData(BuffType buff) const;
	/// <summary>
	/// バフデータのリストを返す
	/// </summary>
	const std::vector<BuffData>& GetBuffs() const { return _buffs; }
private:

	/// <summary>
	/// バフの初期化
	/// </summary>
	void InitBuff();
	/// <summary>
	/// バフの残り時間を更新
	/// </summary>
	void UpdateBuff();


	std::weak_ptr<Player> _owner;

	std::vector<BuffData> _buffs;	// バフと詳細

	std::shared_ptr<PlayerBuffGaugeDrawer> _gaugeDrawer;
};

