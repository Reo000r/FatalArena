#include "EnemyNormal.h"
#include "Player.h"
#include "Animator.h"
#include "WeaponEnemy.h"
#include "Input.h"
#include "Collider.h"
#include "Rigidbody.h"
#include "Calculation.h"
#include "SoundManager.h"
#include <cassert>

#include <DxLib.h>
#include <algorithm>

namespace {
	constexpr float kScaleMul = 2.75f;							// 拡大倍率
	const Vector3 kModelScale = Vector3(1.2f, 1, 1.2f) * kScaleMul;	// モデル拡大倍率
	constexpr float kHitPoint = 500.0f;							// HP
	constexpr float kChaseSpeed = 8.0f * kScaleMul;				// 追いかける速度
	constexpr float kTurnSpeed = 0.06f;							// 回転速度(ラジアン)
	constexpr float kAttackRange = 175.0f * kScaleMul;			// 攻撃に移行する距離
	constexpr float kGround = 0.0f;								// (地面の高さ)
	constexpr int kReactCooltimeFrame = 30;						// 無敵時間

	constexpr float kAttackPower = 200.0f;						// 攻撃力
	constexpr int kAddScore = 1000;								// 加算スコア
	
	constexpr float kChaseDist = 1200.0f;						// 追い始める距離

	// 当たり判定のパラメータ
	const float kColRadius = 60.0f * kModelScale.x;		// 半径
	const float kColHeight = 200.0f * kModelScale.y;	// 身長
	const Vector3 kColOffset = Vector3Up() * (kColHeight - kColRadius);

	const std::wstring kAnimName = L"Armature|Animation_";
	const std::wstring kAnimNameSpawn = kAnimName + L"Emote";
	const std::wstring kAnimNameIdle = kAnimName + L"Idle";
	const std::wstring kAnimNameChase = kAnimName + L"Chase";
	const std::wstring kAnimNameAttack = kAnimName + L"Attack";
	const std::wstring kAnimNameDamage = kAnimName + L"React";
	const std::wstring kAnimNameDeath = kAnimName + L"Dying";

	constexpr float kBaseAnimSpeed = 1.0f;
	constexpr float kDamageAnimSpeed = 1.7f;
	constexpr float kSpawnAnimEndRatio = 0.5f;		// 開始時のスケーリングを終わらせるタイミング
	
	// 武器データ
	const std::wstring kHandFrameName = L"mixamorig:RightHandIndex1";
	const std::wstring kWeaponModelPath = L"data/model/weapon/EnemyWeapon.mv1";

	const Vector3 kWeaponOffsetPos = Vector3Up();					// 位置補正
	const Vector3 kWeaponOffsetScale = Vector3(1.0f, 1.3f, 2.0f) * 1.2f;	// 拡縮補正

	//const float kWeaponRad = 70.0f * kWeaponOffsetScale.x;		// 武器半径
	const float kWeaponRad = 250.0f * kWeaponOffsetScale.x;		// 武器半径(バグ応急処置)
	const float kWeaponDist = 300.0f * kWeaponOffsetScale.y;	// 武器長さ

	// 角度補正
	const Vector3 kWeaponOffsetDir = Vector3(
		Calc::ToRadian(60.0f),
		Calc::ToRadian(90.0f),
		Calc::ToRadian(50.0f));

	// 武器の当たり判定を切り替えるタイミング
	constexpr float kAttackColStart = 0.1f;	// 当たり判定を付け始める
	constexpr float kAttackColEnd = 0.6f;	// 当たり判定を切る
}

EnemyNormal::EnemyNormal(int modelHandle) :
	EnemyBase(CapsuleColliderDesc(kColRadius, kColOffset),
		kHitPoint, kAttackRange),
	_nowUpdateState(&EnemyNormal::UpdateSpawning),
	_weapon(std::make_unique<WeaponEnemy>())

{
	rigidbody->Init(true);

	// モデルの読み込み
	_animator->Init(modelHandle);
	MV1SetScale(_animator->GetModelHandle(), Vector3(0,0,0));

	// 使用するアニメーションを全て入れる
	_animator->SetAnimData(kAnimNameSpawn,	kBaseAnimSpeed, false);
	_animator->SetAnimData(kAnimNameIdle, kBaseAnimSpeed, true);
	_animator->SetAnimData(kAnimNameChase,	kBaseAnimSpeed, true);
	_animator->SetAnimData(kAnimNameAttack, kBaseAnimSpeed, false);
	_animator->SetAnimData(kAnimNameDamage, kDamageAnimSpeed, false);
	_animator->SetAnimData(kAnimNameDeath,	kBaseAnimSpeed, false);
	// 最初のアニメーションを設定する
	_animator->SetStartAnim(kAnimNameSpawn);

	//MV1SetScale(_animator->GetModelHandle(), kModelScale);

	
}

EnemyNormal::~EnemyNormal()
{
	// modelはanimator側で消している
}

void EnemyNormal::Init(std::weak_ptr<Player> player, std::weak_ptr<Physics> physics)
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

	MV1SetRotationXYZ(_animator->GetModelHandle(), Vector3(0, _rotAngle + Calc::ToRadian(180.0f), 0));



	// 武器の初期化
	// モデル読み込み
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

	// physicsに登録
	EntryPhysics(physics);
	_weapon->EntryPhysics(physics);

	// 武器位置更新
	WeaponUpdate();
}

void EnemyNormal::Update()
{
	_animator->Update();

	// 状態遷移確認
	CheckStateTransition();

	// 現在のステートに応じたUpdateが行われる
	(this->*_nowUpdateState)();

	WeaponUpdate();
}

void EnemyNormal::Draw()
{
	// 当たり判定を行ってからモデルの位置を設定する
	MV1SetPosition(_animator->GetModelHandle(), GetPos());
	// モデルの描画
	MV1DrawModel(_animator->GetModelHandle());

	_weapon->Draw();
}

float EnemyNormal::GetMaxHitPoint() const
{
	return kHitPoint;
}

float EnemyNormal::GetAttackPower() const
{
	return kAttackPower;
}

void EnemyNormal::OnCollide(const std::weak_ptr<Collider> collider)
{
	// coliderと衝突
}

void EnemyNormal::TakeDamage(float damage, std::shared_ptr<Collider> attacker)
 {
	// 出現中と死亡状態ではダメージを受けない
	if (_nowUpdateState == &EnemyNormal::UpdateSpawning ||
		_nowUpdateState == &EnemyNormal::UpdateDeath) return;

	// HPを減らす
	_hitPoint -= damage;

	// プレイヤーの方向を向く
	if (!_player.expired()) {
		// プレイヤーへの方向ベクトル
		Vector3 dirToPlayer = (_player.lock()->GetPos() - GetPos());
		if (dirToPlayer.SqrMagnitude() > 0.0f) {
			// Y軸回転角度を計算
			_rotAngle = atan2f(dirToPlayer.x, dirToPlayer.z);
		}
	}

	MV1SetRotationXYZ(_animator->GetModelHandle(), Vector3(0, _rotAngle + Calc::ToRadian(180.0f), 0));


	// 死亡判定
	if (_hitPoint <= 0.0f &&
		_state == State::Active) {
		_state = State::Dying;
		_nowUpdateState = &EnemyNormal::UpdateDeath;
		_animator->ChangeAnim(kAnimNameDeath, false);
		_player.lock()->AddScore(kAddScore);		//スコア加算
		SoundManager::GetInstance().PlaySoundType(SEType::Attack2);
		SoundManager::GetInstance().PlaySoundType(SEType::EnemyDeath);
		// 物理判定から除外する
		ReleasePhysics();
		_weapon->ReleasePhysics();
		return;
	}

	SoundManager::GetInstance().PlaySoundType(SEType::Attack1);

	// 既に被弾状態ならアニメーションを最初から再生
	if (_nowUpdateState == &EnemyNormal::UpdateDamage) {
		auto& animData = _animator->FindAnimData(kAnimNameDamage);
		animData.frame = 0.0f;
		animData.isEnd = false;
	}
	// そうでなければ被弾状態へ遷移
	else {
		_nowUpdateState = &EnemyNormal::UpdateDamage;
		_animator->ChangeAnim(kAnimNameDamage, false);
	}
}

void EnemyNormal::CheckStateTransition()
{
	// 出現状態か判定(優先)
	if (_nowUpdateState == &EnemyNormal::UpdateSpawning) {
		// アニメーションが終了したら、追跡状態に移行
		if (_animator->IsEnd(kAnimNameSpawn)) {
			float dist = (GetPos() - _player.lock()->GetPos()).Magnitude();
			if (dist <= kChaseDist) {
				// プレイヤーを追い始める
				_state = State::Active;
				_nowUpdateState = &EnemyNormal::UpdateChase;
				_animator->ChangeAnim(kAnimNameChase, true);
			}
			else {
				// 待機する
				_state = State::Active;
				_nowUpdateState = &EnemyNormal::UpdateIdle;
				_animator->ChangeAnim(kAnimNameIdle, true);
			}
		}
		return; // 出現中は他の状態に遷移しない
	}

	// 死亡したか判定(優先)
	if (_hitPoint <= 0.0f &&
		_state == State::Active) {
		_state = State::Dying;
		_nowUpdateState = &EnemyNormal::UpdateDeath;
		_animator->ChangeAnim(kAnimNameDeath, false);
		// 物理判定から除外する
		ReleasePhysics();
		_weapon->ReleasePhysics();
		return;	// 死亡した場合は他の状態に遷移しない
	}

	// 割り込み不可の状態判定
	// 死亡または被弾アニメーション中は他の状態に遷移しない
	if (_nowUpdateState == &EnemyNormal::UpdateDeath) {
		return;
	}
	if (_nowUpdateState == &EnemyNormal::UpdateDamage) {
		// 被弾アニメーションが終了していない場合はこのまま
		if (!_animator->IsEnd(kAnimNameDamage)) {
			return;
		}
	}
	// 攻撃中の場合は移行しない
	if (_nowUpdateState == &EnemyNormal::UpdateAttack &&
		!_animator->IsEnd(kAnimNameAttack)) {
		return;
	}

	// プレイヤー情報の確認
	if (_player.expired()) {
		// もしプレイヤーがいない場合、待機状態に戻る
		if (_nowUpdateState != &EnemyNormal::UpdateIdle) {
			_nowUpdateState = &EnemyNormal::UpdateIdle;
			_animator->ChangeAnim(kAnimNameIdle, true);
		}
		return;
	}

	// プレイヤーとの距離
	float distance = (_player.lock()->GetPos() - GetPos()).Magnitude();

	// 攻撃状態
	// プレイヤーとの距離が攻撃移行範囲よりも近かったら
	if (distance <= _transferAttackRad) {
		// 剣の攻撃状態をリセット
		_weapon->ResetAttackState();
		SoundManager::GetInstance().PlaySoundType(SEType::EnemyAttack);

		// プレイヤーの方向を向く
		if (!_player.expired()) {
			// プレイヤーへの方向ベクトル
			Vector3 dirToPlayer = (_player.lock()->GetPos() - GetPos());
			if (dirToPlayer.SqrMagnitude() > 0.0f) {
				// Y軸回転角度を計算
				_rotAngle = atan2f(dirToPlayer.x, dirToPlayer.z);
			}
		}
		MV1SetRotationXYZ(_animator->GetModelHandle(), Vector3(0, _rotAngle + Calc::ToRadian(180.0f), 0));


		// アニメーションが終了した直後でも、再度Attackに遷移できるように
		// _nowUpdateState != &EnemyNormal::UpdateAttack の条件を外しても良いが、
		// アニメーションの再アタッチコストを考えると、このままの方が効率的
		if (_nowUpdateState != &EnemyNormal::UpdateAttack) {
			_nowUpdateState = &EnemyNormal::UpdateAttack;
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
	// プレイヤーが追跡範囲内にいれば追跡する
	if (_nowUpdateState != &EnemyNormal::UpdateChase &&
		distance <= kChaseDist) {
		_nowUpdateState = &EnemyNormal::UpdateChase;
		_animator->ChangeAnim(kAnimNameChase, true);
		return;
	}

	// それ以外の場合、待機状態に戻る
	if (_nowUpdateState != &EnemyNormal::UpdateIdle &&
		distance > kChaseDist) {
		_nowUpdateState = &EnemyNormal::UpdateIdle;
		_animator->ChangeAnim(kAnimNameIdle, true);
	}
}

void EnemyNormal::UpdateSpawning()
{
	// 出現アニメーション中は移動を止める
	rigidbody->SetVel(Vector3());

	// アニメーションの進行度に合わせてスケールを変更する
	auto& animData = _animator->FindAnimData(kAnimNameSpawn);
	if (animData.totalFrame > 0.0f) {
		// 進行度を計算 (0.0 ~ 1.0)
		float progress = 0.0f;
		// (0除算回避)
		float totalFrame = animData.totalFrame * kSpawnAnimEndRatio;
		if (std::min<float>(animData.frame, totalFrame)) {
			progress = std::min<float>(animData.frame / totalFrame, 1.0f);
		}
		// スケールを線形補間
		MV1SetScale(_animator->GetModelHandle(), kModelScale * progress);
	}
}

void EnemyNormal::UpdateIdle()
{
	// プレイヤーの方向を向く
	RotateToPlayer();
}

void EnemyNormal::UpdateChase()
{
	// プレイヤーの方向を向く
	RotateToPlayer();

	// 前方に移動
	Vector3 vel = rigidbody->GetDir() * kChaseSpeed;
	rigidbody->SetVel(vel);
}

void EnemyNormal::UpdateAttack()
{
	// 攻撃中は移動を止める
	rigidbody->SetVel(Vector3());


	// 攻撃アニメーションの情報を取得
	auto& animData = _animator->FindAnimData(kAnimNameAttack);

	const bool prevHit = _weapon->IsHit();

	// アニメーションの総フレーム数が0より大きい場合かつ
	// まだ当たっていない場合のみ処理
	if (animData.totalFrame > 0.0f && 
		!prevHit) {
		// 現在の再生フレームと総フレームから進行度を計算(0.0f-1.0f)
		float progress = animData.frame / animData.totalFrame;

		// 進行度が指定の範囲内である場合攻撃
		if (progress >= kAttackColStart && progress <= kAttackColEnd) {
			// 当たり判定が行われていない場合は
			// 状態をリセット
			if (!_weapon->GetCollisionState()) {
				_weapon->ResetAttackState();
			}
			_weapon->SetCollisionState(true);	// 当たり判定を有効にする
		}
		else {
			_weapon->SetCollisionState(false);
		}
	}
	else {
		// もしアニメーションが再生されていない、または終了している場合は
		// 当たり判定を無効にする
		_weapon->SetCollisionState(false);
	}
}

void EnemyNormal::UpdateDamage()
{
	// 移動を停止する
	rigidbody->SetVel(Vector3());
}

void EnemyNormal::UpdateDeath()
{
	// 死亡アニメーションが終了したら、更新を止める
	if (_animator->IsEnd(kAnimNameDeath) && _state != State::Dead) {
		_state = State::Dead; // 状態を死亡完了にする
	}
}

void EnemyNormal::RotateToPlayer()
{
	if (_player.expired()) return;

	// プレイヤーへの方向ベクトル
	Vector3 dirToPlayer = (_player.lock()->GetPos() - GetPos());
	if (dirToPlayer.SqrMagnitude() == 0.0f) return; // 距離がゼロなら何もしない
	dirToPlayer.Normalized();

	// Y軸回転角度を計算
	float targetAngle = atan2f(dirToPlayer.x, dirToPlayer.z);

	// 現在の角度から目標角度までの差分を計算
	float diff = Calc::RadianNormalize(targetAngle - _rotAngle);

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

void EnemyNormal::WeaponUpdate()
{
	// 武器をアタッチするフレームの番号を検索
	int frameIndex = MV1SearchFrame(_animator->GetModelHandle(), kHandFrameName.c_str());
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
