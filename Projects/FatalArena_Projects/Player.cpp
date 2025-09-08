#include "Player.h"
#include "Camera.h"
#include "Animator.h"
#include "WeaponPlayer.h"
#include "Input.h"
#include "Collider.h"
#include "ColliderData.h"
#include "Rigidbody.h"
#include "Calculation.h"
#include "ItemFactory.h"
#include "PlayerBuffManager.h"
#include "PlayerReinforcementManager.h"
#include "GameManager.h"
#include "Arena.h"
#include "SoundManager.h"
#include "Physics.h"
#include <cassert>
#include <algorithm>

#include <DxLib.h>

namespace {
	// 当たり判定のパラメータ
	constexpr float kColRadius = 70.0f; // 半径
	constexpr float kColHeight = 350.0f; // 身長
	// カプセルの始点(足元)から終点(頭頂部)までのベクトル
	const Vector3 kColOffset = Vector3Up() * (kColHeight - kColRadius * 2.0f);
	
	constexpr float kAttackPower = 100.0f;
	constexpr float kWalkSpeed = 7.0f;
	constexpr float kDashSpeed = 20.0f;
	//constexpr float kJumpForce = 20.0f;
	constexpr float kGround = 0.0f;
	constexpr int kReactCooltimeFrame = 60;	// 無敵時間

	constexpr float kTurnSpeed = 0.20f;	// 回転速度(ラジアン)
	const float kStartPlayerRotAmount = Calc::ToRadian(180.0f);	// 初期の向き(ラジアン)
	
	constexpr float kMaxHitPoint = 500.0f;
	constexpr float kMaxStamina = 100.0f;
	constexpr float kStaminaCooltimeFrame = 30.0f;		// スタミナが回復し始めるまでにかかる時間
	constexpr float kStaminaRecoveryFrame = 120.0f;		// スタミナが最大回復までにかかる時間
	constexpr float kStaminaRecoveryAmount = 1.0f / kStaminaRecoveryFrame * kMaxStamina;	// 1f当たりのスタミナ回復量
	constexpr float kStaminaDecreaceAmount = 20.0f;		// スタミナ減少量

	const std::wstring kAnimName = L"Armature|Animation_";
	const std::wstring kAnimNameIdle =			kAnimName + L"Idle";
	const std::wstring kAnimNameWalk =			kAnimName + L"Walk";
	const std::wstring kAnimNameRun =			kAnimName + L"Run";
	const std::wstring kAnimNameAttackNormal =	kAnimName + L"Attack360High";
	const std::wstring kAnimNameAttackBack =	kAnimName + L"AttackBackhand";
	const std::wstring kAnimNameAttackCombo1 =	kAnimName + L"AttackCombo1";
	const std::wstring kAnimNameAttackCombo2 =	kAnimName + L"AttackCombo2";
	const std::wstring kAnimNameAttackCombo3 =	kAnimName + L"AttackCombo3";
	const std::wstring kAnimNameSpecialAttack1 = kAnimName + L"SpecialAttack1";
	const std::wstring kAnimNameSpecialAttack2 = kAnimName + L"SpecialAttack2";
	const std::wstring kAnimNameBlock =			kAnimName + L"Block";
	//const std::wstring kAnimNameBlockReact =	kAnimName + L"BlockReact";
	const std::wstring kAnimNameReact =			kAnimName + L"BlockReact";
	const std::wstring kAnimNameBuff =			kAnimName + L"Buff";
	const std::wstring kAnimNameDead =			kAnimName + L"Dying";
	const std::wstring kAnimNameAppeal =		kAnimName + L"WinAnim";

	constexpr float kBaseAnimSpeed = 1.0f;
	constexpr float kAttackAnim1Speed = 0.7f;
	constexpr float kAttackAnim2Speed = 0.8f;
	constexpr float kAttackAnim3Speed = 0.8f;
	constexpr float kDeadAnimSpeed = 0.7f;

	// 攻撃入力受付
	constexpr float kAttackCombo1InputStart = 0.05f;
	constexpr float kAttackCombo1InputEnd	= 1.0f;
	constexpr float kAttackCombo2InputStart = 0.05f;
	constexpr float kAttackCombo2InputEnd	= 1.0f;

	// 攻撃判定を切り替えるタイミング
	//constexpr float kAttackCombo1Start	= 0.5f;
	//constexpr float kAttackCombo1End	= 1.0f;
	//constexpr float kAttackCombo2Start	= 0.3f;
	//constexpr float kAttackCombo2End	= 1.0f;
	//constexpr float kAttackCombo3Start	= 0.3f;
	//constexpr float kAttackCombo3End	= 0.6f;
	constexpr float kAttackCombo1Start	= 0.6f;
	constexpr float kAttackCombo1End	= 0.95f;
	constexpr float kAttackCombo2Start	= 0.4f;
	constexpr float kAttackCombo2End	= 0.95f;
	constexpr float kAttackCombo3Start	= 0.3f;
	constexpr float kAttackCombo3End	= 0.6f;

	constexpr float kAttackCombo1SoundTiming = 0.45f;
	constexpr float kAttackCombo2SoundTiming = 0.1f;
	constexpr float kAttackCombo3SoundTiming = 0.1f;


	// 武器データ
	const Vector3 kWeaponOffsetPos = Vector3Up();							// 位置補正
	const Vector3 kWeaponOffsetScale = Vector3(1.0f, 1.3f, 2.0f) * 1.2f;	// 拡縮補正

	const float kWeaponRad = 50.0f * kWeaponOffsetScale.x;		// 武器半径
	const float kWeaponDist = 200.0f * kWeaponOffsetScale.y;	// 武器長さ

	// 角度補正
	const Vector3 kWeaponOffsetDir = Vector3(
		Calc::ToRadian(60.0f),
		Calc::ToRadian(90.0f),
		Calc::ToRadian(50.0f));
}

Player::Player() :
	Collider(PhysicsData::Priority::Middle,
		PhysicsData::GameObjectTag::Player,
		PhysicsData::ColliderKind::Capsule,
		false, true),
	_nowUpdateState(&Player::UpdateIdle),
	_animator(std::make_unique<Animator>()),
	_camera(),
	_weapon(std::make_unique<WeaponPlayer>()),
	_buffManager(),
	_rotAngle(kStartPlayerRotAmount),
	_quaternion(),
	_hasDerivedAttackInput(false),
	_hitPoint(kMaxHitPoint),
	_stamina(kMaxStamina),
	_staminaRecoveryStandbyFrame(0),
	_isAlive(true),
	_reactCooltime(0),
	_isPlayAttackSound(false)
{
	// データ設定
	StatsData data;
	data.maxHealth = kMaxHitPoint;
	data.maxStamina = kMaxStamina;
	data.maxStrength = kAttackPower;
	PlayerReinforcementManager::SetStatsData(data);

	rigidbody->Init(true);

	// 当たり判定データ設定
	CapsuleColliderDesc desc;
	desc.radius = kColRadius;
	desc.startToEnd = kColOffset;
	colliderData = CreateColliderData(
		desc,	// 種別
		false,	// isTrigger
		true	// isCollision
	);

	// 自身の武器とは当たり判定を行わない
	colliderData->AddThroughTag(PhysicsData::GameObjectTag::PlayerAttack);

	// モデルの読み込み
	_animator->Init(MV1LoadModel(L"data/model/character/Player.mv1"));
	MV1SetScale(_animator->GetModelHandle(), Vector3(1, 1, 1) * 2.0f);

	// 使用するアニメーションを全て入れる
	_animator->SetAnimData(kAnimNameIdle,			kBaseAnimSpeed, true);
	_animator->SetAnimData(kAnimNameWalk,			kBaseAnimSpeed, true);
	_animator->SetAnimData(kAnimNameRun,			kBaseAnimSpeed, true);
	_animator->SetAnimData(kAnimNameAttackNormal,	kBaseAnimSpeed, false);
	_animator->SetAnimData(kAnimNameAttackBack,		kBaseAnimSpeed, false);
	_animator->SetAnimData(kAnimNameAttackCombo1,	kAttackAnim1Speed, false, kAttackCombo1InputStart, kAttackCombo1InputEnd);
	_animator->SetAnimData(kAnimNameAttackCombo2,	kAttackAnim2Speed, false, kAttackCombo2InputStart, kAttackCombo2InputEnd);
	_animator->SetAnimData(kAnimNameAttackCombo3,	kAttackAnim3Speed, false);
	_animator->SetAnimData(kAnimNameSpecialAttack1, kBaseAnimSpeed, false);
	_animator->SetAnimData(kAnimNameSpecialAttack2, kBaseAnimSpeed, false);
	_animator->SetAnimData(kAnimNameBlock,			kBaseAnimSpeed, true);
	//_animator->SetAnimData(kAnimNameBlockReact,		kBaseAnimSpeed, false);
	_animator->SetAnimData(kAnimNameReact,			kBaseAnimSpeed, false);
	_animator->SetAnimData(kAnimNameBuff,			kBaseAnimSpeed, false);
	_animator->SetAnimData(kAnimNameDead,			kDeadAnimSpeed, false);
	_animator->SetAnimData(kAnimNameAppeal,			kBaseAnimSpeed, false);
	// 最初のアニメーションを設定する
	_animator->SetStartAnim(kAnimNameIdle);
}

Player::~Player()
{
	// modelはanimator側で消している
}

void Player::Init(std::weak_ptr<Camera> camera, std::weak_ptr<Physics> physics, 
	std::weak_ptr<PlayerBuffManager> playerBuffManager)
{
	_camera = camera;
	_buffManager = playerBuffManager;
	MV1SetRotationXYZ(_animator->GetModelHandle(), Vector3(0, _rotAngle, 0));


	// 武器初期化
	int weaponModelHandle = MV1LoadModel(L"data/model/weapon/PlayerWeapon.mv1");
	assert(weaponModelHandle >= 0 && "モデルハンドルが正しくない");
	_weapon->Init(
		weaponModelHandle,
		kWeaponRad,			// 当たり判定半径
		kWeaponDist,		// 当たり判定長さ
		kWeaponOffsetPos,	// 位置補正
		kWeaponOffsetScale,	// 拡縮補正
		kWeaponOffsetDir	// 角度補正
	);

	// 武器に自分自身と自分の攻撃力を設定
	_weapon->SetOwnerStatus(shared_from_this());

	EntryPhysics(physics);
	_weapon->EntryPhysics(physics);

	// 武器位置更新
	WeaponUpdate();
}

void Player::Update()
{
	_animator->Update();

	// 死んでいないなら
	if (IsAlive()) {
		// 状態遷移確認
		CheckStateTransition();

		// 現在のステートに応じたUpdateが行われる
		(this->*_nowUpdateState)();

		// 無敵時間更新
		if (_reactCooltime > 0) _reactCooltime--;
	}

	// 武器更新
	WeaponUpdate();
}

void Player::Draw()
{
	// 当たり判定を行ってからモデルの位置を設定する
	MV1SetPosition(_animator->GetModelHandle(), GetPos());
	// モデルの描画
	MV1DrawModel(_animator->GetModelHandle());

	_weapon->Draw();

#ifdef _DEBUG
	//int color = 0xffffff;
	//int y = 16 * 7;
	//DrawFormatString(0, y, color, L"Player:Pos (%.3f,%.3f,%.3f)", GetPos().x, GetPos().y, GetPos().z);
	//y += 16;
	//DrawFormatString(0, y, color, L"Player:Vel (%.3f,%.3f,%.3f)", GetVel().x, GetVel().y, GetVel().z);
	//y += 16;
	//DrawFormatString(0, y, color, L"Player:RotAngle (%.3f)", _rotAngle);
#endif
}

void Player::OnCollide(const std::weak_ptr<Collider> collider)
{
	// coliderと衝突
}

float Player::GetMaxHitPoint()
{
	auto data = PlayerReinforcementManager::GetStatsData();
	return (data.maxHealth * data.maxHealthMag);
}

float Player::GetMaxStamina() const
{
	auto data = PlayerReinforcementManager::GetStatsData();
	return (data.maxStamina * data.maxStaminaMag);
}

float Player::GetAttackPower() const
{
	auto data = PlayerReinforcementManager::GetStatsData();
	float power = data.maxStrength * data.maxStrengthMag;
	if (_buffManager.lock()->GetData(BuffType::Strength).isActive) {
		power *= _buffManager.lock()->GetData(BuffType::Strength).amount;
	}
	return power;
}

void Player::TakeDamage(float damage, std::shared_ptr<Collider> attacker)
{
	// 死亡状態、または攻撃中はダメージを受け付けない
	if (_nowUpdateState == &Player::UpdateDeath ||
		_nowUpdateState == &Player::UpdateAttackFirst ||
		_nowUpdateState == &Player::UpdateAttackSecond ||
		_nowUpdateState == &Player::UpdateAttackThird) {
		return;
	}

	// 無敵時間外であれば
	if (_reactCooltime <= 0) {
		// HPを減らす
		_hitPoint -= damage;
		//_reactCooltime = kReactCooltimeFrame;
		SoundManager::GetInstance().PlaySoundType(SEType::PlayerReact);

		// 武器の当たり判定を無効化
		_weapon->SetCollisionState(false);
		// 攻撃してきた相手の方向を向く
		if (attacker) {
			// 相手への方向ベクトルを計算
			Vector3 dirToAttacker = (GetPos() - attacker->GetPos());
			// 距離が0でなければ
			if (dirToAttacker.SqrMagnitude() > 0.0f) {
				// Y軸の回転角度を計算し、モデルの向きに反映する
				_rotAngle = atan2f(dirToAttacker.x, dirToAttacker.z);
				MV1SetRotationXYZ(_animator->GetModelHandle(), Vector3(0, _rotAngle, 0));
			}
		}
		// hpが0以下の場合は死亡
		if (_hitPoint <= 0.0f) {
			if (_nowUpdateState != &Player::UpdateDeath) {
				_nowUpdateState = &Player::UpdateDeath;
				_animator->ChangeAnim(kAnimNameDead, false);
				SoundManager::GetInstance().PlaySoundType(SEType::PlayerDeath);
			}
			return;
		}

		// 既に被弾状態ならアニメーションを最初から再生
		if (_nowUpdateState == &Player::UpdateDamage) {
			auto& animData = _animator->FindAnimData(kAnimNameReact);
			animData.frame = 0.0f;
			animData.isEnd = false;
		}
		// そうでなければ被弾状態へ遷移
		else {
			_nowUpdateState = &Player::UpdateDamage;
			_animator->ChangeAnim(kAnimNameReact, false);
			_hasDerivedAttackInput = false;		// 攻撃コンボをリセット
		}
	}
}

void Player::AddScore(int addScore)
{
	int finalAddScore = addScore;
	if (_buffManager.lock()->GetData(BuffType::ScoreBoost).isActive) {
		finalAddScore = static_cast<int>(finalAddScore * _buffManager.lock()->GetData(BuffType::ScoreBoost).amount);
	}
	GameManager::GetInstance().AddEnemyDefeatScore(finalAddScore);	// スコア加算
}

void Player::Heal(float amount)
{
	float healAmount = std::min<float>(amount, GetMaxHitPoint() - _hitPoint);
	_hitPoint += healAmount;
}

void Player::CheckStateTransition()
{
	// 死亡判定を最優先
	// hpが0以下の場合は死亡
	if (_hitPoint <= 0.0f) {
		if (_nowUpdateState != &Player::UpdateDeath) {
			_nowUpdateState = &Player::UpdateDeath;
			_animator->ChangeAnim(kAnimNameDead, false);
			// 武器の当たり判定を無効化
			_weapon->SetCollisionState(false);
		}
		return;
	}

	// 割り込み不可の状態判定
	if (_nowUpdateState == &Player::UpdateDeath) {
		return; // 死亡中は他の状態に遷移しない
	}
	if (_nowUpdateState == &Player::UpdateDamage) {
		// 被弾アニメーションが終了していなければ、他の状態に遷移しない
		if (!_animator->IsEnd(kAnimNameReact)) {
			return;
		}
	}

	Input& input = Input::GetInstance();
	Vector3 stick = input.GetPadLeftSitck();

	// 現在再生中のアニメーションが終了しているか
	bool isEndAnim = _animator->IsEnd(_animator->GetCurrentAnimName());

	// 攻撃中かどうか
	bool isAttack =
		(_nowUpdateState == &Player::UpdateAttackFirst ||
		_nowUpdateState == &Player::UpdateAttackSecond ||
		_nowUpdateState == &Player::UpdateAttackThird);

	if (isAttack) {
		// 攻撃アニメーションが終了した場合にのみ次の遷移を判断する
		if (isEndAnim) {
			// 派生入力があったかつスタミナが減らせるか
			if (_hasDerivedAttackInput &&
				CanStaminaDecreace()) {
				// スタミナを減らす
				StaminaDecreace();
				// 攻撃効果音を鳴らしていない状態にする
				_isPlayAttackSound = false;
				// 攻撃の度に入力方向を測定し、入力があればその方向に向かせる
				// スティック入力があるか
				bool stickInputState = (stick.Magnitude() >= 0.005f);

				// スティック入力がないかつ
				// スティックではない入力があった場合、そちらを優先する
				if (!stickInputState && (
					input.IsPress("Gameplay:Up") ||
					input.IsPress("Gameplay:Down") ||
					input.IsPress("Gameplay:Left") ||
					input.IsPress("Gameplay:Right")))
				{
					// xが横、zが縦
					stick = Vector3();
					if (input.IsPress("Gameplay:Up"))		stick += Vector3(0, 0, -1);
					if (input.IsPress("Gameplay:Down"))		stick += Vector3(0, 0, +1);
					if (input.IsPress("Gameplay:Left"))		stick += Vector3(-1, 0, 0);
					if (input.IsPress("Gameplay:Right"))	stick += Vector3(+1, 0, 0);

					stick.Normalized();
				}

				// 入力があった場合のみキャラクターの向きを変更
				if (stick.x != 0.0f || stick.z != 0.0f) {
					// カメラの向きを考慮しつつ目標の角度を計算
					const float cameraRot = _camera.lock()->GetRotAngleY();
					float targetAngle = atan2f(stick.z, stick.x) + -cameraRot + DX_PI_F * 0.5f;

					// 現在の角度から目標角度までの最短差分を計算
					float diff = targetAngle - _rotAngle;

					// 更新
					_rotAngle += diff;

					// 現在の角度も正規化しておく
					Calc::RadianNormalize(_rotAngle);
					// 適用
					MV1SetRotationXYZ(_animator->GetModelHandle(), Vector3(0, _rotAngle, 0));
				}
				// どの攻撃からの派生か
				if (_nowUpdateState == &Player::UpdateAttackFirst) {
					_nowUpdateState = &Player::UpdateAttackSecond;
					_animator->ChangeAnim(kAnimNameAttackCombo2, false);
					_hasDerivedAttackInput = false;
					return; // 遷移したので処理終了
				}
				else if (_nowUpdateState == &Player::UpdateAttackSecond) {
					_nowUpdateState = &Player::UpdateAttackThird;
					_animator->ChangeAnim(kAnimNameAttackCombo3, false);
					_hasDerivedAttackInput = false;
					return; // 遷移したので処理終了
				}
			}

			// 派生入力がなければ
			// 武器の当たり判定を無効化
			_weapon->SetCollisionState(false);
			// スティックが一定以上傾いているなら
			if (CanRunInput()) {
				_nowUpdateState = &Player::UpdateDash;
				_animator->ChangeAnim(kAnimNameRun, true);
			}
			else if (CanWalkInput()) {
				_nowUpdateState = &Player::UpdateWalk;
				_animator->ChangeAnim(kAnimNameWalk, true);
			}
			else {
				_nowUpdateState = &Player::UpdateIdle;
				_animator->ChangeAnim(kAnimNameIdle, true);
			}
			return;
		}

		// 攻撃中は何もしない
		return;
	}

	// 各種条件を一度ローカル変数に格納する
	const bool canAttack = CanAttackInput();
	const bool canDecreaseStamina = CanStaminaDecreace();
	const bool isGrounded = GetPos().y <= kGround;
	const bool canWalk = CanWalkInput();
	const bool canRun = CanRunInput();

	// 攻撃開始
	// 攻撃入力があったかつ
	// スタミナが減らせるなら
	if (canAttack &&
		canDecreaseStamina) {
		_nowUpdateState = &Player::UpdateAttackFirst;
		_animator->ChangeAnim(kAnimNameAttackCombo1, false);
		_hasDerivedAttackInput = false;
		StaminaDecreace();
		// 攻撃効果音を鳴らしていない状態にする
		_isPlayAttackSound = false;
		return;
	}

	// ダッシュ
	// スティックが一定以上傾いているなら
	if (isGrounded &&
		canRun) {
		if (_nowUpdateState != &Player::UpdateDash) {
			_nowUpdateState = &Player::UpdateDash;
			_animator->ChangeAnim(kAnimNameRun, true);
			// 武器の当たり判定を無効化
			_weapon->SetCollisionState(false);
		}
		return;
	}

	// 歩行
	// スティックの入力があるなら
	if (isGrounded &&
		canWalk) {
		if (_nowUpdateState != &Player::UpdateWalk) {
			_nowUpdateState = &Player::UpdateWalk;
			_animator->ChangeAnim(kAnimNameWalk, true);
			// 武器の当たり判定を無効化
			_weapon->SetCollisionState(false);
		}
		return;
	}

	// 待機
	// それ以外なら
	if (isGrounded) {
		if (_nowUpdateState != &Player::UpdateIdle) {
			_nowUpdateState = &Player::UpdateIdle;
			_animator->ChangeAnim(kAnimNameIdle, true);
			// 武器の当たり判定を無効化
			_weapon->SetCollisionState(false);
		}
		return;
	}
}

void Player::WeaponUpdate()
{
	// 位置更新

	// 手の行列を武器のワールド行列とする
	std::wstring handName = L"mixamorig:RightHandThumb3";
	// 武器をアタッチするフレームの番号を検索
	int frameIndex = MV1SearchFrame(_animator->GetModelHandle(), handName.c_str());
	// インデックスが有効かチェック
	if (frameIndex < 0) {
		assert(false && "指定されたフレームが見つからなかった");
		return;
	}
	// 手のフレームのワールド行列を取得
	Matrix4x4 handWorldMatrix = MV1GetFrameLocalWorldMatrix(
		_animator->GetModelHandle(),frameIndex);

	// 手のワールド行列を渡す
	_weapon->Update(handWorldMatrix);



	// 判定更新

	// 攻撃状態でないならreturn
	if (!(_nowUpdateState == &Player::UpdateAttackFirst ||
		_nowUpdateState == &Player::UpdateAttackSecond ||
		_nowUpdateState == &Player::UpdateAttackThird)) {
		return;
	}

	std::wstring animName = L"";
	float start = 0.0f;
	float end = 0.0f;

	if (_nowUpdateState == &Player::UpdateAttackFirst) {
		animName = kAnimNameAttackCombo1;
		start = kAttackCombo1Start;
		end = kAttackCombo1End;
	}
	else if (_nowUpdateState == &Player::UpdateAttackSecond) {
		animName = kAnimNameAttackCombo2;
		start = kAttackCombo2Start;
		end = kAttackCombo2End;
	}
	else {
		animName = kAnimNameAttackCombo3;
		start = kAttackCombo3Start;
		end = kAttackCombo3End;
	}

	// 攻撃アニメーションの情報を取得
	auto& animData = _animator->FindAnimData(animName);

#ifdef _DEBUG
	//float prog = animData.frame / animData.totalFrame;
	//int color = 0xffffff;
	//int y = 16 * 12;
	//DrawFormatString(0, y, color, 
	//	L"Player:AttackAnimProgress : %.3f", 
	//	prog);
#endif // _DEBUG

	
	// アニメーションの総フレーム数が0より大きい場合のみ処理
	if (animData.totalFrame > 0.0f) {
		// 現在の再生フレームと総フレームから進行度を計算(0.0f - 1.0f)
		float progress = animData.frame / animData.totalFrame;

		// 進行度が指定の範囲内である場合攻撃
		if (progress >= start && progress <= end) {
			// 当たり判定が行われていない場合は
			// 攻撃リストをリセット
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

void Player::UpdateIdle()
{
	// 移動を停止する
	rigidbody->SetVel(Vector3());

	// スタミナ回復処理
	StaminaRecovery();
}

void Player::UpdateWalk()
{
	// 移動処理
	Move(kWalkSpeed);
	// 進行方向への方向転換処理
	Rotate();

	// スタミナ回復処理
	StaminaRecovery();
}

void Player::UpdateDash()
{
	// 移動処理
	Move(kDashSpeed);
	// 進行方向への方向転換処理
	Rotate();

	// スタミナ回復処理
	StaminaRecovery();
}

void Player::UpdateAttackFirst()
{
	// アニメーションの現在のフレームを取得
	float currentFrame = _animator->GetCurrentAnimFrame();
	// 現在のアニメーションデータから入力受付期間を取得
	const Animator::AnimData& currentAnimData = _animator->FindAnimData(_animator->
		GetCurrentAnimName());
	
	// 入力受付期間内かつ
	// 再生された瞬間ではないかつ
	// 攻撃ボタンが押されたら
	// 次の攻撃へ派生可能にする
	if (currentAnimData.animName == kAnimNameAttackCombo1 &&
		currentAnimData.frame != currentAnimData.animSpeed*1.1f &&
		currentFrame >= currentAnimData.inputAcceptanceStartFrame &&
		currentFrame <= currentAnimData.inputAcceptanceEndFrame &&
		CanAttackInput())
	{
		_hasDerivedAttackInput = true;
	}

	if (!_isPlayAttackSound &&
		currentAnimData.animName == kAnimNameAttackCombo1 &&
		currentAnimData.frame / currentAnimData.totalFrame >= kAttackCombo1SoundTiming) {
		SoundManager::GetInstance().PlaySoundType(SEType::Swing1);
		_isPlayAttackSound = true;
	}
}

void Player::UpdateAttackSecond()
{
	// アニメーションの現在のフレーム
	float currentFrame = _animator->GetCurrentAnimFrame();
	// 現在のアニメーションデータ(入力受付期間取得用)
	const Animator::AnimData& currentAnimData = 
		_animator->FindAnimData(_animator->GetCurrentAnimName());

	// 入力受付期間内かつ、攻撃ボタンが押されたら次の攻撃へ派生可能にする
	if (currentAnimData.animName == kAnimNameAttackCombo2 &&
		currentFrame >= currentAnimData.inputAcceptanceStartFrame &&
		currentFrame <= currentAnimData.inputAcceptanceEndFrame &&
		CanAttackInput())
	{
		_hasDerivedAttackInput = true;
	}

	if (!_isPlayAttackSound &&
		currentAnimData.animName == kAnimNameAttackCombo2 &&
		currentAnimData.frame / currentAnimData.totalFrame >= kAttackCombo2SoundTiming) {
		SoundManager::GetInstance().PlaySoundType(SEType::Swing1);
		_isPlayAttackSound = true;
	}
}

void Player::UpdateAttackThird()
{
	// 派生は不要

	// 現在のアニメーションデータ(入力受付期間取得用)
	const Animator::AnimData& currentAnimData =
		_animator->FindAnimData(_animator->GetCurrentAnimName());

	if (!_isPlayAttackSound &&
		currentAnimData.animName == kAnimNameAttackCombo3 &&
		currentAnimData.frame / currentAnimData.totalFrame >= kAttackCombo3SoundTiming) {
		SoundManager::GetInstance().PlaySoundType(SEType::Swing2);
		_isPlayAttackSound = true;
	}
}

void Player::UpdateDamage()
{
	// 移動を停止する
	rigidbody->SetVel(Vector3());

	// スタミナ回復処理
	StaminaRecovery();

	// UpdateIdleと同一でないと認識させるためのダミー
	volatile int dummy = 0;
}

void Player::UpdateDeath()
{
	// 死亡アニメーションが終了したら更新を止める
	if (_animator->IsEnd(kAnimNameDead)) {
		// 物理判定から除外する
		ReleasePhysics();

		// 死亡アニメーションが終わったら死亡したことにする
		_isAlive = false;
	}
}

void Player::Move(const float speed)
{
	Vector3 dir = {};
	Vector3 vel = GetVel();
	Position3 pos = GetPos();	// 移動予定位置
	Input& input = Input::GetInstance();

	const float cameraRot = _camera.lock()->GetRotAngleY() * -1;
	
	// スティックによる平面移動
	Vector3 stick = Input::GetInstance().GetPadLeftSitck();

	// スティック入力があるか
	bool stickInputState = (stick.Magnitude() >= 0.005f);

	// スティック入力がないかつ
	// スティックではない入力があった場合、そちらを優先する
	if (!stickInputState && (
		input.IsPress("Gameplay:Up") ||
		input.IsPress("Gameplay:Down") ||
		input.IsPress("Gameplay:Left") ||
		input.IsPress("Gameplay:Right")))
	{
		// xが横、zが縦
		stick = Vector3();
		if (input.IsPress("Gameplay:Up"))		stick += Vector3(0, 0, -1);
		if (input.IsPress("Gameplay:Down"))		stick += Vector3(0, 0, +1);
		if (input.IsPress("Gameplay:Left"))		stick += Vector3(-1, 0, 0);
		if (input.IsPress("Gameplay:Right"))	stick += Vector3(1, 0, 0);

		stick.Normalized();
	}

	// 入力が入っていない時でもxに-0.0fが入っている
	dir.x = -stick.x;
	dir.z = stick.z;

	dir.Normalized();

	// カメラから見た移動方向に変換する
	_quaternion = AngleAxis(Vector3Up(), cameraRot);	// Y軸回転Qを作成
	dir = RotateVector3(_quaternion, dir);				// 回転Qを適用

	vel.x += dir.x * speed;
	vel.z += dir.z * speed;

	pos += vel;
	if (pos.y < kGround) {
		pos.y = kGround;
		vel.y = 0.0f;
	}

	// rigidbodyに編集した移動量を代入
	//rigidbody->SetVel(vel);


	// 現在の位置と速度から次のフレームの予測位置を計算
	Position3 nextPos = GetPos() + vel;

	// 予測位置の原点からのXZ平面上の距離を計算
	Vector2 xzPos(nextPos.x, nextPos.z);
	float distanceFromOrigin = xzPos.Magnitude();

	// 指定の距離を超えていたら制限する
	const float kMaxDistance = Arena::GetArenaRadius() - kColRadius;
	if (distanceFromOrigin > kMaxDistance) {
		// 予測位置を指定の円周上に補正
		Vector2 normalized = xzPos.Normalize();
		nextPos.x = normalized.x * kMaxDistance;
		nextPos.z = normalized.y * kMaxDistance;
	}

	// 現在位置から補正後の予測位置へ向かう新しい速度を計算し、設定する
	Vector3 newVel = nextPos - GetPos();
	rigidbody->SetVel(newVel);
}

void Player::Rotate() {
	Input& input = Input::GetInstance();
	// スティックによる平面移動
	Vector3 stick = input.GetPadLeftSitck();
	// スティック入力があるか
	bool stickInputState = (stick.Magnitude() >= 0.005f);

	// スティック入力がないかつ
	// スティックではない入力があった場合、そちらを優先する
	if (!stickInputState && (
		input.IsPress("Gameplay:Up") ||
		input.IsPress("Gameplay:Down") ||
		input.IsPress("Gameplay:Left") ||
		input.IsPress("Gameplay:Right")))
	{
		// xが横、zが縦
		stick = Vector3();
		if (input.IsPress("Gameplay:Up"))		stick += Vector3(0, 0, -1);
		if (input.IsPress("Gameplay:Down"))		stick += Vector3(0, 0, +1);
		if (input.IsPress("Gameplay:Left"))		stick += Vector3(-1, 0, 0);
		if (input.IsPress("Gameplay:Right"))	stick += Vector3(+1, 0, 0);

		stick.Normalized();
	}

	// 入力があった場合のみキャラクターの向きを変更
	if (stick.x != 0.0f || stick.z != 0.0f) {
		// カメラの向きを考慮しつつ目標の角度を計算
		const float cameraRot = _camera.lock()->GetRotAngleY();
		float targetAngle = atan2f(stick.z, stick.x) + -cameraRot + DX_PI_F * 0.5f;

		// 現在の角度から目標角度までの最短差分を計算
		float diff = targetAngle - _rotAngle;

		// 角度の正規化
		diff = Calc::RadianNormalize(diff);
		// 1フレームで回転できる最大量に制限する
		float turnAmount = std::clamp(diff, -kTurnSpeed, kTurnSpeed);

		// 更新
		_rotAngle += turnAmount;

		// 現在の角度も正規化しておく
		Calc::RadianNormalize(_rotAngle);
		// 適用
		MV1SetRotationXYZ(_animator->GetModelHandle(), Vector3(0, _rotAngle, 0));
	}
}

bool Player::CanAttackInput()
{
	Input& input = Input::GetInstance();
	return (input.IsTrigger("Gameplay:Attack") ||
			input.IsTriggerMouseLeftClick());
}

bool Player::CanWalkInput()
{
	auto& input = Input::GetInstance();

	// スティック入力があるか
	bool stickInputState = (input.GetPadLeftSitck().Magnitude() >= 0.005f);

	// スティック入力か移動キー入力があればtrue
	return (stickInputState ||
		input.IsPress("Gameplay:Up") ||
		input.IsPress("Gameplay:Down") ||
		input.IsPress("Gameplay:Left") ||
		input.IsPress("Gameplay:Right"));
}

bool Player::CanRunInput()
{
	auto& input = Input::GetInstance();

	// 一定以上のスティック入力があるか
	bool stickInputState = (input.GetPadLeftSitck().Magnitude() >= 1000 * 0.8f);

	// スティック入力か移動キー入力があればtrue
	return (stickInputState ||
		input.IsPress("Gameplay:Up") ||
		input.IsPress("Gameplay:Down") ||
		input.IsPress("Gameplay:Left") ||
		input.IsPress("Gameplay:Right"));
}

void Player::StaminaRecovery()
{
	const float maxStamina = GetMaxStamina();

	// スタミナが最大の場合はreturn
	if (_stamina >= maxStamina) return;

	// 回復待機時間更新
	if (_staminaRecoveryStandbyFrame > 0) _staminaRecoveryStandbyFrame--;

	// 回復待機時間がない場合は回復
	if (_staminaRecoveryStandbyFrame <= 0) {
		// 回復量
		float recoveryAmount = std::min<float>(
			kStaminaRecoveryAmount, maxStamina - _stamina);
		_stamina += recoveryAmount;
	}
}

void Player::StaminaDecreace()
{
	// スタミナが減少量より少ない場合は減らさない
	if (!CanStaminaDecreace()) {
		return;
	}

	_stamina -= kStaminaDecreaceAmount;
	_staminaRecoveryStandbyFrame = static_cast<int>(kStaminaCooltimeFrame);
}

bool Player::CanStaminaDecreace()
{
	// スタミナが減少量より少ない場合は減らさない
	return (_stamina >= kStaminaDecreaceAmount);
}

