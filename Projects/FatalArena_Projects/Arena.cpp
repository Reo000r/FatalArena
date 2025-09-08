#include "Arena.h"
#include "Camera.h"
#include "Animator.h"
#include "WeaponPlayer.h"
#include "Input.h"
#include "Collider.h"
#include "ColliderData.h"
#include "Rigidbody.h"
#include "Physics.h"

#include <cassert>
#include <DxLib.h>

namespace {
	const std::wstring kModelPath = L"data/model/field/Arena.mv1";
	const Vector3 kModelScale = Vector3(1.5f, 1.5f, 1.5f) * 0.5f;

	const Vector3 kStartToEnd = Vector3(0.0f, 1000.0f, 0.0f);
	constexpr int kColInnerRadius = 2240;	// 内側の半径
	constexpr int kColOuterRadius = 3240;	// 外側の半径

}

Arena::Arena() :
	Collider(PhysicsData::Priority::Static, 
		PhysicsData::GameObjectTag::SystemWall, 
		PhysicsData::ColliderKind::InvertedCylinder, 
		false, true)
{
	rigidbody->Init(false);

	// 当たり判定データ設定
	InvertedCylinderColliderDesc desc;
	desc.startToEnd = kStartToEnd;
	desc.innerRadius = kColInnerRadius;
	desc.outerRadius = kColOuterRadius;
	colliderData = CreateColliderData(
		desc,	// 種別
		false,	// isTrigger
		true	// isCollision
	);

	// 武器とは当たり判定を行わない
	colliderData->AddThroughTag(PhysicsData::GameObjectTag::PlayerAttack);
	colliderData->AddThroughTag(PhysicsData::GameObjectTag::EnemyAttack);

	// モデルの読み込み
	_modelHandle = MV1LoadModel(kModelPath.c_str());
	MV1SetPosition(_modelHandle, Vector3(0, 0, 0));
	MV1SetScale(_modelHandle, kModelScale);
}

Arena::~Arena()
{
	MV1DeleteModel(_modelHandle);
}

void Arena::Init(std::weak_ptr<Physics> physics)
{
	// physicsに登録
	//EntryPhysics(physics);
}

void Arena::Draw()
{
	MV1DrawModel(_modelHandle);
}

void Arena::OnCollide(const std::weak_ptr<Collider> collider)
{
}

float Arena::GetArenaRadius()
{
	return kColInnerRadius;
}
