#pragma once
#include "Geometry.h"
#include "Collider.h"
#include <memory>

class Camera;
class Animator;
class WeaponPlayer;
class PlayerBuffManager;
class Physics;

/// <summary>
/// 
/// </summary>
class Player final : public Collider {
public:
	Player();
	~Player();

	void Init(std::weak_ptr<Camera> camera, std::weak_ptr<Physics> physics, 
		std::weak_ptr<PlayerBuffManager> playerBuffManager);
	void Update();
	void Draw();

	/// <summary>
	/// 衝突したときに呼ばれる
	/// </summary>
	/// <param name="colider"></param>
	void OnCollide(const std::weak_ptr<Collider> collider) override;

	float GetHitPoint() const { return _hitPoint; }
	static float GetMaxHitPoint();
	float GetStamina() const { return _stamina; }
	float GetMaxStamina() const;
	bool IsAlive() { return _isAlive; }
	float GetAttackPower()const;
	
	/// <summary>
	/// ダメージを受ける処理
	/// </summary>
	/// <param name="damage">受けるダメージ量</param>
	/// <param name="attacker">攻撃してきた相手</param>
	void TakeDamage(float damage, std::shared_ptr<Collider> attacker);

	/// <summary>
	/// バフを加味したスコアを追加する
	/// </summary>
	/// <param name="addScore"></param>
	void AddScore(int addScore);

	void Heal(float amount);

private:
	// UpdateのStateパターン
	// _nowUpdateStateが変数であることを分かりやすくしている
	using UpdateFunc_t = void(Player::*)();
	UpdateFunc_t _nowUpdateState;

private:
	/// <summary>
	/// ステートの遷移条件を確認し、変更可能なステートがあればそれに遷移する
	/// </summary>
	void CheckStateTransition();

	/// <summary>
	/// 武器更新
	/// </summary>
	void WeaponUpdate();

	void UpdateIdle();
	void UpdateWalk();
	void UpdateDash();
	void UpdateAttackFirst();
	void UpdateAttackSecond();
	void UpdateAttackThird();
	void UpdateDamage();
	void UpdateDeath();

	/// <summary>
	/// 移動処理
	/// </summary>
	void Move(const float speed);
	/// <summary>
	/// 進行方向への方向転換処理
	/// </summary>
	void Rotate();

	/// <summary>
	/// 攻撃可能な入力であるか
	/// </summary>
	/// <returns>可能であればtrue</returns>
	bool CanAttackInput();
	/// <summary>
	/// 歩ける入力であるか
	/// </summary>
	/// <returns>スティック入力か移動キー入力があればtrue</returns>
	bool CanWalkInput();
	/// <summary>
	/// 走れる入力であるか
	/// </summary>
	/// <returns>走れる程のスティック入力か移動キー入力があればtrue</returns>
	bool CanRunInput();

	/// <summary>
	/// スタミナの回復待機時間を減らす
	/// 待機時間がない場合は回復させる
	/// </summary>
	void StaminaRecovery();

	/// <summary>
	/// スタミナを減少量分減らす
	/// </summary>
	/// <returns>減らせたかどうか</returns>
	void StaminaDecreace();
	/// <summary>
	/// スタミナを減少量分減らせるかどうか
	/// </summary>
	/// <returns>減らせるかどうか</returns>
	bool CanStaminaDecreace();

	std::unique_ptr<Animator> _animator;

	std::weak_ptr<Camera> _camera;

	std::shared_ptr<WeaponPlayer> _weapon;
	std::weak_ptr<PlayerBuffManager> _buffManager;

	float _rotAngle;
	//Matrix4x4 _rotMtx;
	Quaternion _quaternion;

	// 攻撃の派生入力があったかどうか
	bool  _hasDerivedAttackInput;

	float _hitPoint;	// HP
	float _stamina;		// スタミナ
	int _staminaRecoveryStandbyFrame;		// スタミナ回復待機時間
	bool _isAlive;

	int _reactCooltime;

	bool _isPlayAttackSound;
};

