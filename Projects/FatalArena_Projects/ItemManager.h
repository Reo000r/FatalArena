#pragma once
#include "PlayerBuffManager.h"

#include <memory>
#include <vector>

class ItemBase;
class Physics;
enum class ItemType;

class ItemManager
{
public:
	ItemManager();
	~ItemManager();

	/// <summary>
	/// 初期化
	/// </summary>
	void Init(std::weak_ptr<Physics> physics, 
		std::weak_ptr<PlayerBuffManager> _manager);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 指定されたアイテムを生成する
	/// </summary>
	/// <param name="type"></param>
	void SpawnItem(const BuffType type);

private:
	/// <summary>
	/// 消滅処理が終了したアイテムをリストから削除する
	/// </summary>
	void CleanupDestroyedItems();
	/// <summary>
	/// すべてのアイテムの消滅処理を始める
	/// </summary>
	void StartDestroyAllItems();

private:
	// 生成したアイテム
	std::vector<std::shared_ptr<ItemBase>> _items;

	// 生成する際に必要な情報
	std::weak_ptr<PlayerBuffManager> _manager;
	std::weak_ptr<Physics> _physics;

};

