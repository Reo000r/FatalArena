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

	constexpr float kAudienceNumMul = 0.8f;		// 観客数倍率

	const std::vector<int> kDrawAmount = {
		static_cast<int>(72 * kAudienceNumMul), 
		static_cast<int>(84 * kAudienceNumMul),
		static_cast<int>(96 * kAudienceNumMul),
		static_cast<int>(108 * kAudienceNumMul),
	};
	constexpr int kPosOffsetAmount = 10;		// 位置補正量

	constexpr float kSpawnRadius = 2800.0f;		// 最も内側の円の半径
	constexpr float kAddSpawnRadius = 400.0f;	// 円の半径補正量
	constexpr float kSpawnHeight = 700.0f;		// 生成高度
	constexpr float kAddSpawnHeight = 250.0f;	// 生成高度補正量
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
	float radius = kSpawnRadius;		// 生成する円周の半径
	float spawnHeight = kSpawnHeight;	// 生成高度
	for (const int num : kDrawAmount) {
		float audDistAngle = static_cast<float>(Calc::ToRadian(360) / num);
		for (int i = 0; i < num; ++i) {
			// 周囲に観客を生成
			// spawnRadius内にランダムな位置を計算
			float angle = audDistAngle * i;
			// 原点を中心に生成
			Position3 spawnPos = Vector3(cos(angle) * radius, spawnHeight, sin(angle) * radius);
			Position3 spawnPosOffset = 
				Vector3(GetRand(kPosOffsetAmount - 1), GetRand(kPosOffsetAmount - 1), GetRand(kPosOffsetAmount - 1));
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

