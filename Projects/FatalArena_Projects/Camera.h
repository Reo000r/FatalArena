#pragma once
#include "Geometry.h"
#include <memory>

class Player;

/// <summary>
/// ゲームシーンにおけるカメラを管理するクラス
/// </summary>
class Camera final {
public:
	Camera();
	~Camera();

	void Init(std::weak_ptr<Player> player);
	void Update();
	void Draw() const;

	Position3 GetPos() const { return _pos; }

	/// <summary>
	/// Y軸の回転情報
	/// </summary>
	/// <returns></returns>
	float GetRotAngleY() const { return _rotAngle.y; }

	/// <summary>
	/// 呼ばれたらステートを一つ進める
	/// </summary>
	void AdvanceState();

	bool IsCompleteStartAnimation() const { return _isCompleteStartAnimation; }
	bool IsCompleteEndAnimation() const { return _isCompleteEndAnimation; }

	/// <summary>
	/// アニメーション中かどうか
	/// animFrameが0ならfalse、それ以外はtrue
	/// </summary>
	/// <returns></returns>
	bool IsAnimationInProgress() const { return (_animFrame); }
	/// <summary>
	/// ゲームシーンのフェードアウトが可能かどうか
	/// </summary>
	/// <returns></returns>
	bool CanFadeout() const;

private:
	// UpdateのStateパターン
	// _nowUpdateStateが変数であることを分かりやすくしている
	using UpdateFunc_t = void(Camera::*)();
	UpdateFunc_t _nowUpdateState;

	/// <summary>
	/// 開始時のアニメーション更新
	/// </summary>
	void UpdateStartAnimation();
	/// <summary>
	/// 通常の更新
	/// </summary>
	void UpdateNormal();
	/// <summary>
	/// 終了時のアニメーション更新
	/// </summary>
	void UpdateEndAnimation();

	/// <summary>
	/// カメラを動かす
	/// </summary>
	/// <param name="rotAngle">入力によって補正するカメラの量</param>
	/// <param name="followLerpFactor">線形補間速度</param>
	/// <param name="isEndAnim">終了アニメーション中かどうか</param>
	void CameraUpdate(Vector3 rotAngle, float followLerpFactor, bool isEndAnim);

private:
	Position3 _pos;
	Vector3 _vel;
	// Playerの位置だけ見たい
	std::weak_ptr<Player> _player;
	Position3 _targetPos;

	// それぞれの回転量を表す
	Vector3 _rotAngle;

	float _near;
	float _far;
	float _viewAngle;

	int _lightHandle;

	int _animFrame;						// アニメーションフレーム(0から)
	Position3 _animationPos;			// アニメーション完了後の自身の位置
	Position3 _animationTargetPos;		// アニメーション完了後のターゲットの位置
	bool _isCompleteStartAnimation;		// 開始アニメーションが完了しているか
	bool _isCompleteEndAnimation;		// 終了アニメーションが完了しているか
};

