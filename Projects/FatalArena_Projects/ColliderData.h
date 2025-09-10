#pragma once
#include <list>
#include "ProjectSettings.h"

class ColliderData abstract {
public:
	// コンストラクタ
	ColliderData(PhysicsData::ColliderKind kind_, bool isTrigger_, bool isCollision_) :
		kind(kind_),
		isTrigger(isTrigger_),
		isCollision(isCollision_)
	{
	}

	virtual ~ColliderData() {}

	// getter
	PhysicsData::ColliderKind GetKind() const { return kind; }
	bool IsTrigger() const { return isTrigger; }

	// 当たり判定を無視するタグの追加/削除
	void AddThroughTag(PhysicsData::GameObjectTag tag);
	void RemoveThroughTag(PhysicsData::GameObjectTag tag);

	/// <summary>
	/// 当たり判定を無視するタグかどうか
	/// </summary>
	/// <param name="target">タグ</param>
	/// <returns>無視する場合はtrue</returns>
	bool IsThroughTarget(const PhysicsData::GameObjectTag target) const;

	// 当たり判定を無視するタグのリスト
	std::list<PhysicsData::GameObjectTag>	throughTags;

	// MEMO:ここpublicにしたら問題あるかな
public:

	PhysicsData::ColliderKind	kind;
	bool	isTrigger;
	bool	isCollision;
};

