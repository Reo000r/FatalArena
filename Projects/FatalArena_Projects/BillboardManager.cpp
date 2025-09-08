#include "BillboardManager.h"
#include "BillboardAudience.h"
#include "Calculation.h"

#include <DxLib.h>
#include <string>

namespace {
	// 画像ファイルのパスをここで管理
	const std::vector<std::wstring> kGraphPaths = {
		{ L"data/graph/billboard/Audience1.png" },
		{ L"data/graph/billboard/Audience2.png" },
		{ L"data/graph/billboard/Audience3.png" },
		{ L"data/graph/billboard/Audience4.png" },
		{ L"data/graph/billboard/Audience5.png" },
		{ L"data/graph/billboard/Audience6.png" },
	};

	const std::vector<int> kDrawAmount = {
		24, 28, 32, 36
	};

	constexpr float kSpawnRadius = 2800.0f;
	constexpr float kAddSpawnRadius = 400.0f;
	constexpr float kSpawnHeight = 700.0f;
	constexpr float kAddSpawnHeight = 250.0f;
}

BillboardManager::BillboardManager() :
	_items()
{
	// 処理なし
}

BillboardManager::~BillboardManager()
{
	// 処理なし
}

void BillboardManager::Init()
{
	float radius = kSpawnRadius;	// 外周に生成
	float spawnHeight = kSpawnHeight;	// 外周に生成
	for (const int num : kDrawAmount) {
		float audDistAngle = static_cast<float>(Calc::ToRadian(360) / num);
		for (int i = 0; i < num; ++i) {
			// 周囲に観客を生成
			// spawnRadius内にランダムな位置を計算
			float angle = audDistAngle * i;
			// 原点を中心に生成
			Position3 spawnPos = Vector3(cos(angle) * radius, spawnHeight, sin(angle) * radius);
			int audienceNum = GetRand((int)kGraphPaths.size() - 1);
			// 観客を生成
			SpawnAudience(spawnPos, LoadGraph(kGraphPaths[audienceNum].c_str()));
		}
		radius += kAddSpawnRadius;
		spawnHeight += kAddSpawnHeight;
	}
	
}

void BillboardManager::Update()
{
	for (auto& item : _items) {
		item->Update();
	}
}

void BillboardManager::Draw()
{
	for (auto& item : _items) {
		item->Draw();
	}
}

void BillboardManager::SpawnAudience(Position3 pos, int modelHandle)
{
	float prog = GetRand(10000) / 10000.0f;
	std::shared_ptr<BillboardAudience> newAud =
		std::make_shared<BillboardAudience>(modelHandle, prog);
	newAud->SetPos(pos);
	_items.emplace_back(newAud);
}

