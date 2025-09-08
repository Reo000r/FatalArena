#pragma once
#include "Vector3.h"

#include <memory>
#include <vector>

class BillboardAudience;

class BillboardManager
{
public:
	BillboardManager();
	~BillboardManager();

	/// <summary>
	/// 初期化
	/// </summary>
	void Init();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 指定された観客を生成する
	/// </summary>
	/// <param name="type"></param>
	void SpawnAudience(Position3 pos, int modelHandle);

private:
	// 生成したアイテム
	std::vector<std::shared_ptr<BillboardAudience>> _items;
};

