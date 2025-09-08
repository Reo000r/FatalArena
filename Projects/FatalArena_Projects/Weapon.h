#pragma once
#include "Geometry.h"
#include "Collider.h"
#include <list>

/// <summary>
/// 武器の基底クラス
/// </summary>
class Weapon abstract : public Collider
{
public:
	Weapon(PhysicsData::GameObjectTag tag);
	virtual ~Weapon();

	/// <summary>
	/// 武器初期化
	/// </summary>
	/// <param name="modelHandle">モデルハンドル</param>
	/// <param name="colRad">当たり判定半径</param>
	/// <param name="colHeight">当たり判定の高さ</param>
	/// <param name="transOffset">位置補正</param>
	/// <param name="scale">拡縮補正</param>
	/// <param name="angle">角度補正</param>
	void Init(int modelHandle, float colRad,
		float colHeight,
		Vector3 transOffset = Vector3(),
		Vector3 scale = Vector3(1, 1, 1),
		Vector3 angle = Vector3());

	/// <summary>
	/// 武器更新
	/// </summary>
	/// <param name="parentWorldMatrix">ワールド行列とみなす行列</param>
	void Update(Matrix4x4 parentWorldMatrix);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 武器の所有者と攻撃力を設定する
	/// </summary>
	void SetOwnerStatus(std::weak_ptr<Collider> owner);

	/// <summary>
	/// 当たり判定を行うか切り替える
	/// </summary>
	/// <param name="isCollision"></param>
	void SetCollisionState(bool isCollision);

	bool GetCollisionState();
	bool IsHit();

	void ResetAttackState();

protected:
	// モデルハンドル
	int _modelHandle;

	Vector3 _transOffset;
	Vector3 _rotAngle;
	Vector3 _scale;

	std::weak_ptr<Collider> _owner;

	// 攻撃がヒットした相手のリスト
	std::list<std::weak_ptr<Collider>> _attackedColliders;

	bool _isHit;
};

