#include "Collider.h"
#include "Physics.h"
#include "Rigidbody.h"
#include "ColliderDataSphere.h"
#include "ColliderDataCapsule.h"
#include "ColliderDataInvertedCylinder.h"

#include <cassert>

Collider::Collider(PhysicsData::Priority priority_, PhysicsData::GameObjectTag tag_, 
	PhysicsData::ColliderKind colliderKind, bool isTrigger, bool isCollision) :
	priority(priority_),
	tag(tag_),
	rigidbody(std::make_shared<Rigidbody>()),
	colliderData(nullptr),
	nextPos()
{
}

Collider::~Collider()
{
}

void Collider::EntryPhysics(std::weak_ptr<Physics> physics_)
{
	if (colliderData == nullptr) {
		assert(false && "colliderDataが作られていない");
		return;
	}

	physics = physics_;
	auto thisptr = shared_from_this();
	// Physicsに自身を登録
	physics.lock()->Entry(thisptr);
}

void Collider::ReleasePhysics()
{
	// Physicsから自身を登録解除
	physics.lock()->Release(shared_from_this());
}

Vector3 Collider::GetPos() const
{
	return rigidbody->GetPos();
}

Vector3 Collider::GetVel() const
{
	return rigidbody->GetVel();
}

Vector3 Collider::GetDir() const
{
	return rigidbody->GetDir();
}

std::shared_ptr<ColliderData> Collider::CreateColliderData(SphereColliderDesc desc, bool isTrigger, bool isCollision)
{
	if (colliderData != nullptr) {
		assert(false && "colliderDataは既に作られている");
		return colliderData;
	}

	colliderData = std::make_shared<ColliderDataSphere>(
		isTrigger, isCollision, desc.radius);

	return colliderData;
}

std::shared_ptr<ColliderData> Collider::CreateColliderData(CapsuleColliderDesc desc, bool isTrigger, bool isCollision)
{
	if (colliderData != nullptr) {
		assert(false && "colliderDataは既に作られている");
		return colliderData;
	}

	colliderData = std::make_shared<ColliderDataCapsule>(
		isTrigger, isCollision, desc.radius, desc.startToEnd);

	return colliderData;
}

std::shared_ptr<ColliderData> Collider::CreateColliderData(InvertedCylinderColliderDesc desc, bool isTrigger, bool isCollision)
{
	if (colliderData != nullptr) {
		assert(false && "colliderDataは既に作られている");
		return colliderData;
	}

	colliderData = std::make_shared<ColliderDataInvertedCylinder>(
		isTrigger, isCollision, desc.startToEnd, desc.innerRadius, desc.outerRadius);

	return colliderData;
}

void Collider::SetColliderData(SphereColliderDesc desc, bool isTrigger, bool isCollision)
{
	if (colliderData == nullptr) {
		assert(false && "colliderDataが作られていない");
		return;
	}

	// 共通のデータを変更
	colliderData->isTrigger = isTrigger;
	colliderData->isCollision = isCollision;

	auto colliderDataSphere = std::static_pointer_cast<ColliderDataSphere>(colliderData);
	colliderDataSphere->_radius = desc.radius;
	colliderData = colliderDataSphere;
}

void Collider::SetColliderData(CapsuleColliderDesc desc, bool isTrigger, bool isCollision)
{
	if (colliderData == nullptr) {
		assert(false && "colliderDataが作られていない");
		return;
	}

	// 共通のデータを変更
	colliderData->isTrigger = isTrigger;
	colliderData->isCollision = isCollision;

	auto colliderDataCapsule = std::static_pointer_cast<ColliderDataCapsule>(colliderData);
	colliderDataCapsule->_radius = desc.radius;
	colliderDataCapsule->_startToEnd = desc.startToEnd;
	colliderData = colliderDataCapsule;
}

void Collider::SetColliderData(InvertedCylinderColliderDesc desc, bool isTrigger, bool isCollision)
{
	if (colliderData == nullptr) {
		assert(false && "colliderDataが作られていない");
		return;
	}

	// 共通のデータを変更
	colliderData->isTrigger = isTrigger;
	colliderData->isCollision = isCollision;

	auto colliderDataInvertedCylinder = std::static_pointer_cast<ColliderDataInvertedCylinder>(colliderData);
	colliderDataInvertedCylinder->_startToEnd = desc.startToEnd;
	colliderDataInvertedCylinder->_innerRadius = desc.innerRadius;
	colliderDataInvertedCylinder->_outerRadius = desc.outerRadius;
	colliderData = colliderDataInvertedCylinder;
}
