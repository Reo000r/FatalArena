#pragma once
#include <memory>
#include "Vector3.h"
#include "ProjectSettings.h"

class Rigidbody;
class Physics;
class ColliderData;
class PlayerState;

/// <summary>
/// 当たり判定を持たせたいオブジェクトに継承させる基底クラス
/// </summary>
class Collider abstract : public std::enable_shared_from_this<Collider> {
public:
	/// <summary>
	/// コンストラクタ
	/// 球用
	/// </summary>
	/// <param name="priority">位置補正の優先度</param>
	/// <param name="tag">タグ</param>
	/// <param name="colliderKind">当たり判定種別</param>
	/// <param name="isTrigger"></param>
	/// <param name="isCollision"></param>
	Collider(
		PhysicsData::Priority priority_, 
		PhysicsData::GameObjectTag tag_, 
		PhysicsData::ColliderKind colliderKind, 
		bool isTrigger,
		bool isCollision);

	virtual ~Collider();
	void EntryPhysics(std::weak_ptr<Physics> physics_);
	void ReleasePhysics();

	/// <summary>
	/// 衝突したときに呼ばれる
	/// </summary>
	/// <param name="colider"></param>
	virtual void OnCollide(const std::weak_ptr<Collider> collider) abstract;

	PhysicsData::GameObjectTag GetTag() const	{ return tag; }
	// 位置補正優先度情報を返す
	PhysicsData::Priority GetPriority() const	{ return priority; }
	
	Vector3 GetPos() const;
	Vector3 GetVel() const;
	Vector3 GetDir() const;

	// 各当たり判定の詳細情報

	/// <summary>
	/// 球の当たり判定の詳細情報
	/// </summary>
	struct SphereColliderDesc {
		float radius = 0.0f;
	};
	/// <summary>
	/// カプセルの当たり判定の詳細情報
	/// </summary>
	struct CapsuleColliderDesc {
		float radius = 0.0f;
		Vector3 startToEnd = Vector3();
	};
	/// <summary>
	/// 反転した円柱の当たり判定の詳細情報
	/// </summary>
	struct InvertedCylinderColliderDesc {
		float innerRadius = 0.0f;
		float outerRadius = 0.0f;
		Vector3 startToEnd = Vector3();
	};

protected:
	std::shared_ptr<Rigidbody> rigidbody;
	std::shared_ptr<ColliderData> colliderData;

	std::weak_ptr<Physics> physics;

protected:
	/// <summary>
	/// 球用の当たり判定を作成する
	/// </summary>
	/// <returns></returns>
	std::shared_ptr<ColliderData> CreateColliderData(
		SphereColliderDesc desc, bool isTrigger, bool isCollision);
	/// <summary>
	/// カプセル用の当たり判定を作成する
	/// </summary>
	/// <returns></returns>
	std::shared_ptr<ColliderData> CreateColliderData(
		CapsuleColliderDesc desc, bool isTrigger, bool isCollision);
	/// <summary>
	/// 反転した円柱用の当たり判定を作成する
	/// </summary>
	/// <returns></returns>
	std::shared_ptr<ColliderData> CreateColliderData(
		InvertedCylinderColliderDesc desc, bool isTrigger, bool isCollision);
	
	/// <summary>
	/// 球用の当たり判定を編集する
	/// </summary>
	/// <returns></returns>
	void SetColliderData(
		SphereColliderDesc desc, bool isTrigger, bool isCollision);
	/// <summary>
	/// カプセル用の当たり判定を編集する
	/// </summary>
	/// <returns></returns>
	void SetColliderData(
		CapsuleColliderDesc desc, bool isTrigger, bool isCollision);
	/// <summary>
	/// 反転した円柱用の当たり判定を編集する
	/// </summary>
	/// <returns></returns>
	void SetColliderData(
		InvertedCylinderColliderDesc desc, bool isTrigger, bool isCollision);

private:
	PhysicsData::GameObjectTag	tag;
	// 位置補正優先度情報
	PhysicsData::Priority priority;

private:
	// PhysicsがCollidableを自由に管理するためにフレンド
	friend Physics;

	// 以下はPhysicsのみが扱う型や変数

	// 移動予定位置
	Vector3 nextPos;
};

