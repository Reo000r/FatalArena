#include "ItemManager.h"
#include "ItemBase.h"
#include "ItemFactory.h"
#include "Calculation.h"
#include "Arena.h"
#include <DxLib.h>

namespace {
	const float kSpawnRadius = Arena::GetArenaRadius() - 500.0f;
}

ItemManager::ItemManager() :
	_items(),
	_physics()
{
	// 処理なし
}

ItemManager::~ItemManager()
{
	// 処理なし
}

void ItemManager::Init(std::weak_ptr<Physics> physics,
	std::weak_ptr<PlayerBuffManager> manager)
{
	_physics = physics;
	_manager = manager;
}

void ItemManager::Update()
{
	for (auto& item : _items) {
		item->Update();
	}

	// 消滅処理が終了したアイテムをリストから削除する
	CleanupDestroyedItems();
}

void ItemManager::Draw()
{
	for (const auto& item : _items)	{
		item->Draw();
	}
}

void ItemManager::SpawnItem(const BuffType type)
{
	// spawnRadius内にランダムな位置を計算
	float angle = Calc::ToRadian(static_cast<float>(GetRand(360)));
	//float radius = static_cast<float>(GetRand(static_cast<int>(kSpawnRadius)));
	float radius = static_cast<float>(kSpawnRadius);	// 外周に生成
	// 原点を中心に生成
	Position3 spawnPos = Vector3(cos(angle)*radius, 0.0f, sin(angle)*radius);

	// アイテムを生成し、リストに追加
	std::shared_ptr<ItemBase> newItem = 
		ItemFactory::CreateAndRegister(type, spawnPos, _manager, _physics);
	_items.emplace_back(newItem);
}

void ItemManager::CleanupDestroyedItems()
{
	// 消滅処理が終了したアイテムをvectorの末尾に集めてから削除する
	_items.erase(
		std::remove_if(_items.begin(), _items.end(),
			[](const std::shared_ptr<ItemBase>& item) {
				return (!item->IsAlive());
			}),
		_items.end()
	);
}

void ItemManager::StartDestroyAllItems()
{
	for (auto& item : _items) {
		item->Destroy();
	}
}
