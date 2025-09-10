#include "Camera.h"
#include "Player.h"
#include "Input.h"
#include "Calculation.h"
#include "Statistics.h"
#include "Arena.h"

#include <DxLib.h>

namespace {
	constexpr float kTargetHormingRad = 1000;

	constexpr float kRotSpeedX = 0.03f;
	constexpr float kRotSpeedY = 0.05f;

	// 線形補間速度
	constexpr float kDefaultCameraFollowLerpFactor = 0.1f;		// 通常時の線形補間速度
	constexpr float kStartAnimCameraFollowLerpFactor = 0.000000001f;	// 開始演出時の線形補間速度
	constexpr float kEndAnimCameraFollowLerpFactor = 0.001f;	// 開始演出時の線形補間速度

	const Vector3 kPlayerToTarget = Vector3(0.0f, 250.0f, 0.0f);
	const Vector3 kTargetToCamera = Vector3(0.0f, 350.0f, 700.0f);
	const Vector3 kPlayerToCamera = kTargetToCamera + kPlayerToTarget;

	// 初期位置
	const Vector3 kDefaultPosition = Vector3(
		kTargetToCamera.x, kTargetToCamera.y, kTargetToCamera.z * -1
		) + kPlayerToTarget;
	// 初期回転量
	const float kDefaultRotation = Calc::ToRadian(180.0f);

	constexpr int kStartAnimationStopFrame = 60 * 3.0f;		// 開始アニメーション前の停止フレーム数
	constexpr int kStartAnimationTotalFrame = 60 * 1.0f + kStartAnimationStopFrame;	// 開始アニメーションの総フレーム
	constexpr int kEndAnimationTotalFrame = 60 * 4.0f;		// 終了アニメーションの総フレーム
	const Position3 kStartAnimationPos		= Position3(0.0f, 1000.0f, -0000.0f);
	const Vector3 kStartAnimationTargetPos	= Vector3(0.0f, 1700.0f, 1000.0f);
	//const Position3 kStartAnimationPos		= Position3(0.0f, 100.0f, -800.0f);
	//const Vector3 kStartAnimationTargetPos	= Vector3(0.0f, 1300.0f, 3000.0f);
	const Position3 kEndAnimationPos		= kStartAnimationPos;
	const Vector3 kEndAnimationTargetPos	= kStartAnimationTargetPos;

	constexpr int kCanFadeoutFrameOffset = 20;
}

Camera::Camera() :
	_nowUpdateState(&Camera::UpdateStartAnimation),
	_pos(kStartAnimationPos),
	_player(),
	_targetPos(kPlayerToTarget),
	_rotAngle(Vector3(0.0f, kDefaultRotation, 0.0f)),
	_near(10.0f),
	_far(10000.0f),
	_viewAngle(Calc::ToRadian(60.0f)),
	_lightHandle(-1),
	_animFrame(0),
	_animationPos(),
	_animationTargetPos(),
	_isCompleteStartAnimation(false),
	_isCompleteEndAnimation(false)
{
}

Camera::~Camera()
{
	DeleteLightHandle(_lightHandle);
}

void Camera::Init(std::weak_ptr<Player> player)
{
	_player = player;
	_targetPos = _player.lock()->GetPos() + kPlayerToTarget;
	//SetCameraPositionAndTarget_UpVecY(pos, target);
	
	// 初期位置にカメラを設定
	//CameraUpdate(Vector3(), 1.0f, true);



	float followLerpFactor = 1.0f;
	// 終了地点を目標とする
	Vector3 finalPos = kStartAnimationPos;
	Vector3 finalTargetPos = kStartAnimationTargetPos;
	// 現在位置と最終的にいてほしい位置の中点を求める
	_pos = _pos * (1.0f - followLerpFactor) + finalPos * followLerpFactor;
	_targetPos = _targetPos * (1.0f - followLerpFactor) + finalTargetPos * followLerpFactor;

	// カメラの位置、描画距離、画角を更新
	SetCameraPositionAndTarget_UpVecY(_pos, _targetPos);
	SetCameraNearFar(_near, _far);
	SetupCamera_Perspective(_viewAngle);



	//_lightHandle = CreatePointLightHandle(_pos, _far, 
	//	1.0f, 0.005f, 0.0f);
	_lightHandle = CreateSpotLightHandle(
		_pos,
		Vector3Up(),
		_viewAngle,
		_viewAngle * 0.9f,
		_far,
		1.0f,
		0.005f,
		0.0f);
}

void Camera::Update()
{
	// ステートに応じた更新処理
	(this->*_nowUpdateState)();
}

void Camera::Draw() const
{
	// ライトの位置と方向を更新
	if (_lightHandle != -1) {
		// ライトの位置をカメラの位置に設定
		SetLightPositionHandle(_lightHandle, _pos);

		// ライトの方向をカメラの位置から注視点へのベクトルに設定
		VECTOR direction = (_targetPos - _pos).Normalize();
		SetLightDirectionHandle(_lightHandle, direction);

		// ライトを有効にする
		SetLightEnableHandle(_lightHandle, true);
	}

#ifdef _DEBUG
	//int color = 0xffffff;
	//int y = 16 * 11;
	//DrawFormatString(0, y, color, L"Camera:Pos (%.3f,%.3f,%.3f)", _pos.x, _pos.y, _pos.z);
	//y += 16;
	//Vector3 rotAngle;
	//rotAngle = _rotAngle;
	//DrawFormatString(0, y, color, L"Camera:RotAngle (%.3f,%.3f,%.3f)", rotAngle.x, rotAngle.y, rotAngle.z);
#endif
}

void Camera::AdvanceState()
{
	// ステートを一つ進める
	if (_nowUpdateState == &Camera::UpdateStartAnimation) {
		_nowUpdateState = &Camera::UpdateNormal;
	}
	else if (_nowUpdateState == &Camera::UpdateNormal) {
		_nowUpdateState = &Camera::UpdateEndAnimation;
	}
}

bool Camera::CanFadeout() const
{
	// ステートが終了アニメーション中でない場合不可
	if (_nowUpdateState != &Camera::UpdateEndAnimation) return false;

	// 残り進行度がフェードに必要な時間を下回っているならフェード可能
	// カメラの余分な回転が行われる前に遷移させたいため必要時間に足しフェードを早めている
	bool ret = (kEndAnimationTotalFrame - _animFrame < Statistics::kFadeInterval + kCanFadeoutFrameOffset);
	return ret;
}

void Camera::UpdateStartAnimation()
{
	if (_isCompleteStartAnimation) return;

	_animFrame++;

	if (_animFrame >= kStartAnimationTotalFrame &&
		!_isCompleteStartAnimation) {
		// アニメーションが終了したことを告げる
		_isCompleteStartAnimation = true;
		_animFrame = 0;
		return;
	}

	// 停止フレーム数を超えたら動き始める
	if (_animFrame >= kStartAnimationStopFrame) {
		float ratio = static_cast<float>(_animFrame) / (kStartAnimationTotalFrame);
		float addFollowLerpFactor = kDefaultCameraFollowLerpFactor - kStartAnimCameraFollowLerpFactor;
		float finalFollowLerpFactor = kStartAnimCameraFollowLerpFactor + (addFollowLerpFactor * ratio);

		// アニメーション処理
		CameraUpdate(Vector3(), finalFollowLerpFactor, false);
	}
}

void Camera::UpdateNormal()
{
	Input& input = Input::GetInstance();

	// スティックによる平面移動
	Vector3 stick = input.GetPadRightSitck();

	Position3 mousePosCurrent = input.GetMousePosition();
	Position3 mousePosLast =
		Position3(Statistics::kScreenCenterWidth, 0, Statistics::kScreenCenterHeight);

	// 一定以上動いていればマウスの入力を優先する
	if ((mousePosLast - mousePosCurrent).Magnitude() >= 20.0f) {
		stick = (mousePosLast - mousePosCurrent).Normalize() * 1000.0f;
	}

	// マウスの位置を画面中央に設定
	SetMousePoint(static_cast<int>(Statistics::kScreenCenterWidth),
		static_cast<int>(Statistics::kScreenCenterHeight));

	Vector3 rotAngle = Vector3(0.0f, stick.x * -0.001f * kRotSpeedY, 0.0f);

	CameraUpdate(rotAngle, kDefaultCameraFollowLerpFactor, false);
}

void Camera::UpdateEndAnimation()
{
	if (_isCompleteEndAnimation) return;

	_animFrame++;

	if (_animFrame >= kEndAnimationTotalFrame &&
		!_isCompleteEndAnimation) {
		// アニメーションが終了したことを告げる
		_isCompleteEndAnimation = true;
		return;
	}

	float ratio = static_cast<float>(_animFrame) / kEndAnimationTotalFrame;
	float addFollowLerpFactor = 
		(kDefaultCameraFollowLerpFactor * 0.5f) - kEndAnimCameraFollowLerpFactor;
	float finalFollowLerpFactor = kEndAnimCameraFollowLerpFactor + (addFollowLerpFactor * ratio);

	// アニメーション処理
	CameraUpdate(Vector3(), finalFollowLerpFactor, true);
	
	//CameraUpdate(Vector3(), kEndAnimCameraFollowLerpFactor, true);
}

void Camera::CameraUpdate(Vector3 rotAngle, float followLerpFactor, bool isEndAnim)
{
	Vector3 finalPos;
	Vector3 finalTargetPos;
	// 終了アニメーション中なら
	if (isEndAnim) {
		// 終了地点を目標とする
		finalPos = kEndAnimationPos;
		finalTargetPos = kEndAnimationTargetPos;
	}
	// でなければ
	else {
		// プレイヤーと一定距離離れ、一定回転した場所を目標とする

		// Y軸回転
		_rotAngle += rotAngle;

		Matrix4x4 rotMtx = MatIdentity();

		Matrix4x4 playerRotMtx = MatIdentity();
		// さらにカメラが持つ回転情報を加える
		Matrix4x4 cameraRotMtx = MatRotateY(_rotAngle.y);
		// プレイヤーが持つ回転情報とカメラが持つ回転情報を合わせ
		// 最終的な回転情報を生成する
		rotMtx = playerRotMtx * cameraRotMtx;

		// プレイヤーの移動に合わせて移動を行う
		Vector3 playerPos = _player.lock()->GetPos();
		// プレイヤーからカメラに向かうベクトルを
		// プレイヤーの回転情報に合わせて回転させる
		Vector3 toCamera = rotMtx.VecMultiple(kPlayerToCamera);
		Vector3 toTarget = rotMtx.VecMultiple(kPlayerToTarget);

		// 上下方向はカメラを遅らせて追尾
		// カメラが最終的にいてほしい位置
		finalPos = playerPos + toCamera;
		finalTargetPos = playerPos + toTarget;
	}

	// 現在位置と最終的にいてほしい位置の中点を求める
	_pos = _pos * (1.0f - followLerpFactor) + finalPos * followLerpFactor;
	_targetPos = _targetPos * (1.0f - followLerpFactor) + finalTargetPos * followLerpFactor;

	// 原点からのXZ平面上の距離を計算
	Vector2 xzPos(_pos.x, _pos.z);
	float distanceFromOrigin = xzPos.Magnitude();

	// 指定した距離を超えていたら制限する
	const float kMaxDistance = Arena::GetArenaRadius();
	if (distanceFromOrigin > kMaxDistance) {
		Vector2 normalized = xzPos.Normalize();
		_pos.x = normalized.x * kMaxDistance;
		_pos.z = normalized.y * kMaxDistance;
	}

	// カメラの位置、描画距離、画角を更新
	SetCameraPositionAndTarget_UpVecY(_pos, _targetPos);
	SetCameraNearFar(_near, _far);
	SetupCamera_Perspective(_viewAngle);
}
