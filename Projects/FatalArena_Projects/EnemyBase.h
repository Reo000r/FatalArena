#pragma once
#include "Geometry.h"
#include "Collider.h"
#include <memory>

class Player;
class Animator;
class Physics;
class WeaponEnemy;

class EnemyBase abstract : public Collider
{
public:
	// 敵の状態を示すenum
	enum class State {
		Spawning,	// 生成中
		Active,		// 通常状態(生存)
		Dying,		// 死亡(アニメーション再生中)
		Dead		// 死亡(アニメーション終了)
	};

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="desc"></param>
	/// <param name="hitPoint">HP</param>
	/// <param name="transferAttackRad">攻撃移行範囲</param>
	EnemyBase(CapsuleColliderDesc desc, float hitPoint, float transferAttackRad);
	virtual ~EnemyBase();

	virtual void Init(std::weak_ptr<Player> player, std::weak_ptr<Physics> physics) abstract;
	virtual void Update() abstract;
	virtual void Draw() abstract;

	bool IsAlive() { return (_hitPoint > 0.0f); }

	Matrix4x4 GetRotMtx() const { return _rotMtx; }

	/// <summary>
	/// 現在の状態を返す
	/// </summary>
	/// <returns></returns>
	State GetState() const { return _state; }

	float GetHitPoint() const { return _hitPoint; }
	virtual float GetMaxHitPoint() const abstract;
	virtual float GetAttackPower() const abstract;

	void SetPos(const Vector3& pos);

	/// <summary>
	/// ダメージを受ける処理
	/// </summary>
	/// <param name="damage">受けるダメージ量</param>
	/// <param name="attacker">攻撃してきた相手</param>
	virtual void TakeDamage(float damage, std::shared_ptr<Collider> attacker) abstract;

protected:
	/// <summary>
	/// ステートの遷移条件を確認し、変更可能なステートがあればそれに遷移する
	/// </summary>
	virtual void CheckStateTransition() abstract;

	std::unique_ptr<Animator> _animator;
	
	std::weak_ptr<Player> _player;

	float _rotAngle;
	Matrix4x4 _rotMtx;
	Quaternion _quaternion;

	// HP
	float _hitPoint;

	// 攻撃移行範囲
	float _transferAttackRad;

	// 自身の状態を保持
	State _state;
};
