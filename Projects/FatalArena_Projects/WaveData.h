#pragma once

#include <vector>
#include "Vector3.h"
#include "EnemyFactory.h"

// 1グループの敵の生成情報
struct SpawnInfo {
	EnemyType type = EnemyType::Normal;		// 敵の種類
	int count = 0;							// 生成数
	Position3 basePosition = Position3();	// 生成基準位置
	float spawnRadius = 0.0f;				// この半径内にランダムに配置する
};

// 1ウェーブ分のデータ
struct WaveData {
	std::vector<SpawnInfo> spawnGroups; // このウェーブで生成される敵グループのリスト
};