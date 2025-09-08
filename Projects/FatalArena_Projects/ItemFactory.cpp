#include "ItemFactory.h"
#include "ItemBase.h"
#include "ItemHeal.h"
#include "ItemScoreBoost.h"
#include "ItemStrength.h"
#include "Player.h"
#include "PlayerBuffManager.h"
#include "Calculation.h"

#include <DxLib.h>
#include <cassert>
#include <string>

namespace {
	// モデルファイルのパスをここで管理
	const std::unordered_map<BuffType, std::wstring> kModelPaths = {
		{ BuffType::Heal,		L"data/model/item/Item_BottleA.mv1" },
		{ BuffType::ScoreBoost, L"data/model/item/Item_Coin.mv1" },
		{ BuffType::Strength,	L"data/model/item/Item_Food.mv1" },
	};
	
	// 共通定数
	constexpr float kItemScale = 2.0f;
	const float kColRadius = 50.0f * kItemScale;				// 当たり判定の半径
	const float kSpawnDepthY = -kColRadius - (10.0f * kItemScale);// 生成時の深さ
	const float kTotalAnimFrame = 60.0f * 0.5f;					// 生成/消滅アニメーションフレーム
	const float kModelRotSpeed = Calc::ToRadian(2.0f);			// 回転速度
	const Vector3 kColTransOffset = Vector3(0, kColRadius, 0);
	const Vector3 kModelSize = Vector3(1, 1, 1) * kItemScale;
	const Vector3 kModelAngle = Vector3(Calc::ToRadian(30.0f), 0, 0);

	// ヒール用
	const Vector3 kModelHealTransOffset = Vector3(0, -kColRadius, 0);
	const float kHealTotalFrame = 60.0f * 1.0f;					// 効果時間
	//const float kHealAmount = (Player::GetMaxHitPoint() * 0.4f) / kHealTotalFrame;	// 1f当たりの回復量
	
	// スコア増加用
	const Vector3 kModelScoreBoostTransOffset = Vector3(0, -kColRadius, 0);
	const float kScoreBoostTotalFrame = 60.0f * 10.0f;	// 効果時間
	const float kScoreBoostMulAmount = 1.5f;			// スコア増加倍率

	// 攻撃力増加用
	const Vector3 kModelStrengthTransOffset = Vector3(0, -kColRadius, 0);
	const float kStrengthTotalFrame = 60.0f * 10.0f;	// 効果時間
	constexpr float kStrengthMulAmount = 1.5f;			// 攻撃力増加倍率
}

// staticメンバー変数の実体を定義
std::unordered_map<BuffType, int> ItemFactory::_modelHandles;

void ItemFactory::LoadResources()
{
	// 全てのモデルを読み込み、ハンドルを保存する
	for (const auto& pair : kModelPaths) {
		const BuffType& type = pair.first;
		const std::wstring& path = pair.second;
		int handle = MV1LoadModel(path.c_str());
		assert(handle >= 0 && "モデルの読み込みに失敗");
		_modelHandles[type] = handle;
	}
}

void ItemFactory::ReleaseResources()
{
	// 保存されている全てのモデルハンドルを解放する
	for (const auto& pair : _modelHandles) {
		MV1DeleteModel(pair.second);
	}
	_modelHandles.clear();
}

std::shared_ptr<ItemBase> ItemFactory::CreateAndRegister(
	BuffType type,
	const Vector3& position, 
	std::weak_ptr<PlayerBuffManager> manager,
	std::weak_ptr<Physics> physics)
{
	std::shared_ptr<ItemBase> newItem = nullptr;

	// 複製元のモデルハンドルが存在するかチェック
	auto it = _modelHandles.find(type);
	assert(it != _modelHandles.end() && "要求されたタイプのモデルが読み込まれていない");

	// モデルを複製してハンドルを取得
	// (secondにint型、モデルハンドルが保存されている)
	int duplicatedHandle = MV1DuplicateModel(it->second);
	assert(duplicatedHandle != -1 && "モデルの複製に失敗");

	// 種類に応じて生成する
	switch (type) {
	case BuffType::Heal:
	{
		// ステータス設定
		BuffData data;
		data.type = type;
		data.activeFrame = static_cast<int>(kHealTotalFrame);
		data.maxActiveFrame = static_cast<int>(kHealTotalFrame);
		const float healAmount = (Player::GetMaxHitPoint() * 0.4f) / kHealTotalFrame;	// 1f当たりの回復量
		data.amount = healAmount;
		data.isActive = true;
		auto itemHeal = std::make_shared<ItemHeal>(data, duplicatedHandle, manager);
		
		// 派生先のInitを呼び出す
		itemHeal->Init(kModelHealTransOffset);
		newItem = itemHeal;
		newItem->Init(
			kColRadius,			// 当たり判定半径
			kColTransOffset,	// 位置補正
			kModelSize,			// 拡縮補正
			kModelAngle);		// 角度補正
		break;
	}
	case BuffType::ScoreBoost:
	{
		// ステータス設定
		BuffData data;
		data.type = type;
		data.activeFrame = static_cast<int>(kScoreBoostTotalFrame);
		data.maxActiveFrame = static_cast<int>(kScoreBoostTotalFrame);
		data.amount = kScoreBoostMulAmount;
		data.isActive = true;
		auto itemScoreBoost = std::make_shared<ItemScoreBoost>(data, duplicatedHandle, manager);

		// 派生先のInitを呼び出す
		itemScoreBoost->Init(kModelScoreBoostTransOffset);
		newItem = itemScoreBoost;
		newItem->Init(
			kColRadius,			// 当たり判定半径
			kColTransOffset,	// 位置補正
			kModelSize,			// 拡縮補正
			kModelAngle);		// 角度補正
		break;
	}
	case BuffType::Strength:
	{
		// ステータス設定
		BuffData data;
		data.type = type;
		data.activeFrame = static_cast<int>(kStrengthTotalFrame);
		data.maxActiveFrame = static_cast<int>(kStrengthTotalFrame);
		data.amount = kStrengthMulAmount;
		data.isActive = true;
		auto itemStrength = std::make_shared<ItemStrength>(data, duplicatedHandle, manager);

		// 派生先のInitを呼び出す
		itemStrength->Init(kModelStrengthTransOffset);
		newItem = itemStrength;
		newItem->Init(
			kColRadius,			// 当たり判定半径
			kColTransOffset,	// 位置補正
			kModelSize,			// 拡縮補正
			kModelAngle);		// 角度補正
		break;
	}
	default:
		assert(false && "不明な敵タイプが指定された");
		MV1DeleteModel(duplicatedHandle); // 不要になったハンドルを解放
		return nullptr;
	}

	// 生成した敵の初期化
	if (newItem) {
		// 生成処理
		newItem->Spawn(position, kSpawnDepthY, (int)kTotalAnimFrame, kModelRotSpeed);
		// 当たり判定登録
		newItem->EntryPhysics(physics);
	}
	else {
		// 生成に失敗した場合もハンドルを解放
		MV1DeleteModel(duplicatedHandle);
	}

	return newItem;
}

float ItemFactory::GetStrengthMulAmount()
{
	return kStrengthMulAmount;
}
