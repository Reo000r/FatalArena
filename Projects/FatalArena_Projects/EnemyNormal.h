#pragma once
#include "EnemyBase.h"

/// <summary>
/// 無難な行動を行う敵
/// </summary>
class EnemyNormal final : public EnemyBase
{
public:
	EnemyNormal(int modelHandle);
	~EnemyNormal();

	void Init(std::weak_ptr<Player> player, std::weak_ptr<Physics> physics) override;
	void Update() override;
	void Draw() override;

	float GetMaxHitPoint() const override;
	float GetAttackPower() const override;

	/// <summary>
	/// 衝突したときに呼ばれる
	/// </summary>
	/// <param name="colider"></param>
	void OnCollide(const std::weak_ptr<Collider> collider) override;

	/// <summary>
	/// ダメージを受ける処理
	/// </summary>
	/// <param name="damage">受けるダメージ量</param>
	/// <param name="attacker">攻撃してきた相手</param>
	void TakeDamage(float damage, std::shared_ptr<Collider> attacker) override;


private:
	/// <summary>
	/// ステートの遷移条件を確認し、変更可能なステートがあればそれに遷移する
	/// </summary>
	void CheckStateTransition() override;

	using UpdateFunc_t = void(EnemyNormal::*)();
	UpdateFunc_t _nowUpdateState;

private:

	/// <summary>
	/// 出現
	/// </summary>
	void UpdateSpawning();
	/// <summary>
	/// 対象を追いかける
	/// </summary>
	void UpdateChase();
	/// <summary>
	/// 攻撃
	/// </summary>
	void UpdateAttack();
	/// <summary>
	/// 被弾
	/// </summary>
	void UpdateDamage();
	/// <summary>
	/// 死亡
	/// </summary>
	void UpdateDeath();

	/// <summary>
	/// プレイヤーの方を向く
	/// </summary>
	void RotateToPlayer();


	/// <summary>
	/// 武器の更新
	/// </summary>
	void WeaponUpdate();

	// 武器
	std::shared_ptr<WeaponEnemy> _weapon;
};
