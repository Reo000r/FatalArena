﻿#pragma once
#include "Vector3.h"

namespace PhysicsData {
	/// <summary>
	/// 当たったものの判別を行うためのタグ
	/// </summary>
	enum class GameObjectTag
	{
		None,			// タグなし
		Player,			// プレイヤー
		PlayerAttack,
		Enemy,			// 敵
		EnemyAttack,
		Item,			// アイテム
		SystemWall,		// システム壁
		StepGround,		// 足場の地面
	};

	/// <summary>
	/// 当たり判定種別
	/// </summary>
	enum class ColliderKind {
		Sphere,
		Capsule,
		InvertedCylinder,
	};

	/// <summary>
	/// 位置補正の優先順位
	/// 数値が低い程動きにくい
	/// </summary>
	enum class Priority : int{
		Static,		// 押し出されない
		High,
		Middle,
		Low,
	};

	/// <summary>
	/// 世界の重力
	/// 反映時は加算で計算
	/// </summary>
	const Vector3 Gravity = { 0.0f, -9.81f * 0.1f, 0.0f };
	const Vector3 MaxGravityAccel = Gravity * 15;
	
	// 減速量
	const float decelerationRate = 0.98f;
	// 移動していないとみなされる閾値
	const float sleepThreshold = 0.005f;

	// 当たり判定回数の最大数
	constexpr int kCheckCollideMaxCount = 16;

	// ゼロと見なす許容範囲
	constexpr float kZeroTolerance = 0.00001f;
	// 当たり判定時に押し戻す追加補正量
	// (そのまま戻すとちょうど当たってしまう位置になるため)
	constexpr float kFixPositionOffset = kZeroTolerance;

	// 仮の地面の高さ
	constexpr float kTempGroundHeight = 0.0f;
}
