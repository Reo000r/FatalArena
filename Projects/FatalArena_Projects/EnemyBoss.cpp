#include "EnemyBoss.h"
#include "Player.h"
#include "Animator.h"
#include "WeaponEnemy.h"
#include "Input.h"
#include "Collider.h"
#include "Rigidbody.h"
#include "Calculation.h"
#include <cassert>

#include <DxLib.h>
#include <algorithm>

namespace {
	constexpr float kScaleMul = 4.0f;		// 拡大倍率
	constexpr float kHitPoint = 20.0f;		// HP
	constexpr float kChaseSpeed = 3.0f * kScaleMul;		// 追いかける速度
	constexpr float kTurnSpeed = 0.03f;		// 回転速度(ラジアン)
	constexpr float kAttackRange = 175.0f * kScaleMul;	// 攻撃に移行する距離
	constexpr float kGround = 0.0f;			// (地面の高さ)
	constexpr int kReactCooltimeFrame = 30;	// 無敵時間

	constexpr float kAttackPower = 50.0f;	// 攻撃力

	// 当たり判定のパラメータ
	constexpr float kColRadius = 50.0f * kScaleMul;		// 半径
	constexpr float kColHeight = 200.0f * kScaleMul;	// 身長
	const Vector3 kColOffset = Vector3Up() * (kColHeight - kColRadius);

	const std::wstring kAnimName = L"Armature|Animation_";
	const std::wstring kAnimNameSpawn = kAnimName + L"React";
	const std::wstring kAnimNameChase = kAnimName + L"Chase";
	const std::wstring kAnimNameAttack = kAnimName + L"Attack";
	const std::wstring kAnimNameDamage = kAnimName + L"React";
	const std::wstring kAnimNameDeath = kAnimName + L"Dying";

	constexpr float kBaseAnimSpeed = 1.0f;

	// 武器データ
	const std::wstring kWeaponModelPath = L"data/model/weapon/EnemyWeapon.mv1";
	constexpr float kWeaponRad = 50.0f;
	constexpr float kWeaponDist = 500.0f;

	const Vector3 kWeaponOffsetPos = Vector3Up();							// 位置補正
	const Vector3 kWeaponOffsetScale = Vector3(1.0f, 1.3f, 2.0f) * 0.35f * kScaleMul;	// 拡縮補正
	// 角度補正
	const Vector3 kWeaponOffsetDir = Vector3(
		Calc::ToRadian(60.0f),
		Calc::ToRadian(90.0f),
		Calc::ToRadian(50.0f));
}

EnemyBoss::EnemyBoss(int modelHandle) :
	EnemyBase(CapsuleColliderDesc(kColRadius, kColOffset), 
		kHitPoint, kAttackRange),
	_nowUpdateState(&EnemyBoss::UpdateSpawning),
	_weapon(std::make_unique<WeaponEnemy>())

{
	rigidbody->Init(true);

	// モデルの読み込み
	_animator->Init(modelHandle);

	// 使用するアニメーションを全て入れる
	_animator->SetAnimData(kAnimNameSpawn,	kBaseAnimSpeed, false);
	_animator->SetAnimData(kAnimNameChase,	kBaseAnimSpeed, true);
	_animator->SetAnimData(kAnimNameAttack, kBaseAnimSpeed, false);
	_animator->SetAnimData(kAnimNameDamage, kBaseAnimSpeed, false);
	_animator->SetAnimData(kAnimNameDeath,	kBaseAnimSpeed, false);
	// 最初のアニメーションを設定する
	_animator->SetStartAnim(kAnimNameSpawn);

	MV1SetScale(_animator->GetModelHandle(), Vector3(1, 1, 1) * kScaleMul);



	// 武器の生成と初期化
	// 武器のインスタンスを生成
	_weapon = std::make_unique<WeaponEnemy>();
	// 武器モデルを読み込む
	int weaponModelHandle = MV1LoadModel(kWeaponModelPath.c_str());
	assert(weaponModelHandle != -1 && "武器モデルの読み込みに失敗");
	// 武器を初期化
	_weapon->Init(
		weaponModelHandle,
		kWeaponRad,			// 当たり判定半径
		kWeaponDist,		// 当たり判定長さ
		kWeaponOffsetPos,	// 位置補正
		kWeaponOffsetScale,	// 拡縮補正
		kWeaponOffsetDir	// 角度補正
	);
	// 最初は当たり判定を無効にしておく
	_weapon->SetCollisionState(false);

	// 武器に自分自身と自分の攻撃力を設定
	_weapon->SetOwnerStatus(shared_from_this());
}

EnemyBoss::~EnemyBoss()
{
	// modelはanimator側で消している
}

void EnemyBoss::Init(std::weak_ptr<Player> player, std::weak_ptr<Physics> physics)
{
	_player = player;

	// 生成時にプレイヤーの方向を向く
	if (!_player.expired()) {
		// プレイヤーへの方向ベクトル
		Vector3 dirToPlayer = (_player.lock()->GetPos() - GetPos());
		if (dirToPlayer.SqrMagnitude() > 0.0f) {
			// Y軸回転角度を計算
			_rotAngle = atan2f(dirToPlayer.x, dirToPlayer.z);
		}
	}

	MV1SetRotationXYZ(_animator->GetModelHandle(), Vector3(0, _rotAngle, 0));
}

void EnemyBoss::Update()
{
	_animator->Update();

	// 状態遷移確認
	CheckStateTransition();

	// 現在のステートに応じたUpdateが行われる
	(this->*_nowUpdateState)();

	if (_reactCooltime > 0) _reactCooltime--;

	WeaponUpdate();
}

void EnemyBoss::Draw()
{
	// 当たり判定を行ってからモデルの位置を設定する
	MV1SetPosition(_animator->GetModelHandle(), GetPos());
	// モデルの描画
	MV1DrawModel(_animator->GetModelHandle());
}

float EnemyBoss::GetMaxHitPoint() const
{
	return kHitPoint;
}

float EnemyBoss::GetAttackPower() const
{
	return kAttackPower;
}

void EnemyBoss::OnCollide(const std::weak_ptr<Collider> collider)
{
	// coliderと衝突
}

void EnemyBoss::TakeDamage(float damage, std::shared_ptr<Collider> attacker)
{
	// 出現中と死亡状態ではダメージを受けない
	if (_nowUpdateState == &EnemyBoss::UpdateSpawning ||
		_nowUpdateState == &EnemyBoss::UpdateDeath) return;

	// 無敵時間外であれば
	if (_reactCooltime <= 0) {
		// HPを減らす
		_hitPoint -= damage;
		_reactCooltime = kReactCooltimeFrame;

		// 死亡判定
		if (_hitPoint <= 0.0f &&
			_state == State::Active) {
			_state = State::Dying;
			_nowUpdateState = &EnemyBoss::UpdateDeath;
			_animator->ChangeAnim(kAnimNameDeath, false);
			return;
		}
		// 既に被弾状態ならアニメーションを最初から再生
		if (_nowUpdateState == &EnemyBoss::UpdateDamage) {
			auto& animData = _animator->FindAnimData(kAnimNameDamage);
			animData.frame = 0.0f;
			animData.isEnd = false;
		}
		// そうでなければ被弾状態へ遷移
		else {
			_nowUpdateState = &EnemyBoss::UpdateDamage;
			_animator->ChangeAnim(kAnimNameDamage, false);
		}
	}
}

void EnemyBoss::CheckStateTransition()
{
	// 出現状態か判定(優先)
	if (_nowUpdateState == &EnemyBoss::UpdateSpawning) {
		// アニメーションが終了したら、追跡状態に移行
		if (_animator->IsEnd(kAnimNameSpawn)) {
			_state = State::Active;
			_nowUpdateState = &EnemyBoss::UpdateChase;
			_animator->ChangeAnim(kAnimNameChase, true);
		}
		return; // 出現中は他の状態に遷移しない
	}

	// 死亡したか判定(優先)
	if (_hitPoint <= 0.0f &&
		_state == State::Active) {
		_state = State::Dying;
		_nowUpdateState = &EnemyBoss::UpdateDeath;
		_animator->ChangeAnim(kAnimNameDeath, false);
		return;	// 死亡した場合は他の状態に遷移しない
	}

	// 割り込み不可の状態判定
	// 死亡または被弾アニメーション中は他の状態に遷移しない
	if (_nowUpdateState == &EnemyBoss::UpdateDeath) {
		return;
	}
	if (_nowUpdateState == &EnemyBoss::UpdateDamage) {
		// 被弾アニメーションが終了していない場合はこのまま
		if (!_animator->IsEnd(kAnimNameDamage)) {
			return;
		}
	}
	// 攻撃中の場合は移行しない
	if (_nowUpdateState == &EnemyBoss::UpdateAttack &&
		!_animator->IsEnd(kAnimNameAttack)) {
		return;
	}

	// プレイヤー情報の確認
	if (_player.expired()) {
		// もしプレイヤーがいない場合、追跡状態に戻る
		if (_nowUpdateState != &EnemyBoss::UpdateChase) {
			_nowUpdateState = &EnemyBoss::UpdateChase;
			_animator->ChangeAnim(kAnimNameChase, true);
		}
		return;
	}

	// プレイヤーとの距離
	float distance = (_player.lock()->GetPos() - GetPos()).Magnitude();

	// 攻撃状態
	// プレイヤーとの距離が攻撃移行範囲よりも近かったら
	if (distance <= _transferAttackRad) {
		// アニメーションが終了した直後でも、再度Attackに遷移できるように
		// _nowUpdateState != &EnemyNormal::UpdateAttack の条件を外しても良いが、
		// アニメーションの再アタッチコストを考えると、このままの方が効率的
		if (_nowUpdateState != &EnemyBoss::UpdateAttack) {
			_nowUpdateState = &EnemyBoss::UpdateAttack;
			_animator->ChangeAnim(kAnimNameAttack, false);
		}
		else {
			// アニメーションが終了している場合、再度再生するためにフレームをリセット
			auto& animData = _animator->FindAnimData(kAnimNameAttack);
			if (animData.isEnd) {
				animData.frame = 0.0f;
				animData.isEnd = false;
			}
		}
		return; // 攻撃状態に決定
	}

	// 追跡状態
	// 上記のいずれでもなければ
	if (_nowUpdateState != &EnemyBoss::UpdateChase) {
		_nowUpdateState = &EnemyBoss::UpdateChase;
		_animator->ChangeAnim(kAnimNameChase, true);
	}
}

void EnemyBoss::UpdateSpawning()
{
	// 出現アニメーション中は移動を止める
	rigidbody->SetVel(Vector3());

	// アニメーションの進行度に合わせてスケールを変更する
	auto& animData = _animator->FindAnimData(kAnimNameSpawn);
	if (animData.totalFrame > 0.0f) {
		// 進行度を計算 (0.0 ~ 1.0)
		float progress = 0.0f;
		// (0除算回避)
		if (std::min<float>(animData.frame, animData.totalFrame)) {
			progress = std::min<float>(animData.frame / animData.totalFrame, 1.0f);
		}
		// スケールを線形補間
		float scale = progress * kScaleMul;
		MV1SetScale(_animator->GetModelHandle(), Vector3(1, 1, 1) * scale);
	}
}

void EnemyBoss::UpdateChase()
{
	// プレイヤーの方向を向く
	RotateToPlayer();

	// 前方に移動
	Vector3 vel = rigidbody->GetDir() * kChaseSpeed;
	rigidbody->SetVel(vel);
}

void EnemyBoss::UpdateAttack()
{
	// 攻撃中は移動を止める
	rigidbody->SetVel(Vector3());
}

void EnemyBoss::UpdateDamage()
{
	// 移動を停止する
	rigidbody->SetVel(Vector3());
}

void EnemyBoss::UpdateDeath()
{
	// 死亡アニメーションが終了したら、更新を止める
	if (_animator->IsEnd(kAnimNameDeath) && _state != State::Dead) {
		// 物理判定から除外する
		ReleasePhysics();
		_state = State::Dead; // 状態を死亡完了にする
	}
}

void EnemyBoss::RotateToPlayer()
{
	// プレイヤーが見つからない場合行わない
	if (_player.expired()) return;

	// プレイヤーへの方向ベクトル
	Vector3 dirToPlayer = (_player.lock()->GetPos() - GetPos());
	if (dirToPlayer.SqrMagnitude() == 0.0f) return; // 距離がゼロなら何もしない
	dirToPlayer.Normalized();

	// Y軸回転角度を計算
	float targetAngle = atan2f(dirToPlayer.x, dirToPlayer.z);

	// 現在の角度から目標角度までの差分を計算
#if true
	float diff = targetAngle - _rotAngle;
	// 角度が-180度以下,180度以上にならないように正規化
	while (diff > Calc::ToRadian(180.0f))  diff -= Calc::ToRadian(360.0f);
	while (diff < Calc::ToRadian(-180.0f)) diff += Calc::ToRadian(360.0f);
#else
	float diff = Calc::RadianNormalize(targetAngle - _rotAngle);
#endif

	// 回転量を制限
	float turnAmount = std::clamp<float>(diff, -kTurnSpeed, kTurnSpeed);
	_rotAngle += turnAmount;

	// モデルに回転を適用
	MV1SetRotationXYZ(_animator->GetModelHandle(), Vector3(0, _rotAngle + Calc::ToRadian(180.0f), 0));

	// Rigidbodyの向きも更新
	Vector3 newDir = Vector3(sinf(_rotAngle), 0.0f, cosf(_rotAngle)).Normalize();
	// 速度は維持しない
	// この時点では向き情報が入っている
	rigidbody->SetVel(newDir);
}

void EnemyBoss::WeaponUpdate()
{
	// 手の行列を武器のワールド行列とする
	std::wstring handName = L"mixamorig:TestHand";
	// 武器をアタッチするフレームの番号を検索
	int frameIndex = MV1SearchFrame(_animator->GetModelHandle(), handName.c_str());
	// インデックスが有効かチェック
	if (frameIndex < 0) {
		assert(false && "指定されたフレームが見つからなかった");
		return;
	}
	// 手のフレームのワールド行列を取得
	Matrix4x4 handWorldMatrix = MV1GetFrameLocalWorldMatrix(
		_animator->GetModelHandle(), frameIndex);

	// 手のワールド行列を渡す
	_weapon->Update(handWorldMatrix);
}
