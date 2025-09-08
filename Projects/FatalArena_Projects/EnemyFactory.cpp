#include "EnemyFactory.h"
#include "EnemyBase.h"
#include "EnemyNormal.h"
#include "Player.h"
#include "Physics.h"
#include <DxLib.h>
#include <cassert>
#include <string>

namespace {
	// モデルファイルのパスをここで一元管理
	const std::unordered_map<EnemyType, std::wstring> kModelPaths = {
		{ EnemyType::Normal, L"data/model/character/EnemyNormal.mv1" },
	};
}

// staticメンバー変数の実体を定義
std::unordered_map<EnemyType, int> EnemyFactory::_modelHandles;

void EnemyFactory::LoadResources()
{
	// 全ての敵モデルを読み込み、ハンドルを保存する
	for (const auto& pair : kModelPaths) {
		const EnemyType& type = pair.first;
		const std::wstring& path = pair.second;
		int handle = MV1LoadModel(path.c_str());
		assert(handle != -1 && "モデルの読み込みに失敗");
		_modelHandles[type] = handle;
	}
}

void EnemyFactory::ReleaseResources()
{
	// 保存されている全てのモデルハンドルを解放する
	for (const auto& pair : _modelHandles) {
		MV1DeleteModel(pair.second);
	}
	_modelHandles.clear();
}

std::shared_ptr<EnemyBase> EnemyFactory::CreateAndRegister(
	EnemyType type,
	const Vector3& position,
	std::weak_ptr<Player> player,
	std::weak_ptr<Physics> physics)
{
	std::shared_ptr<EnemyBase> newEnemy = nullptr;

	// 複製元のモデルハンドルが存在するかチェック
	auto it = _modelHandles.find(type);
	assert(it != _modelHandles.end() && "要求された敵タイプのモデルが読み込まれていない");

	// モデルを複製してハンドルを取得
	// (secondにint型、モデルハンドルが保存されている)
	int duplicatedHandle = MV1DuplicateModel(it->second);
	assert(duplicatedHandle != -1 && "モデルの複製に失敗");

	// 敵の種類に応じて生成するクラスを切り替える
	switch (type) {
	case EnemyType::Normal:
	{
		newEnemy = std::make_shared<EnemyNormal>(duplicatedHandle);
		break;
	}
	//case EnemyType::Other:
	//{
	//	newEnemy = std::make_shared<EnemyRanged>(duplicatedHandle);
	//	break;
	//}
	//case EnemyType::Boss:
	//{
	//	newEnemy = std::make_shared<EnemyBoss>(duplicatedHandle);
	//	break;
	//}
	default:
		assert(false && "不明な敵タイプが指定された");
		MV1DeleteModel(duplicatedHandle); // 不要になったハンドルを解放
		return nullptr;
	}

	// 生成した敵の初期化
	if (newEnemy) {
		// 位置設定
		newEnemy->SetPos(position);
		// 派生先のInitを呼び出す
		newEnemy->Init(player, physics);
	}
	else {
		// 生成に失敗した場合もハンドルを解放
		MV1DeleteModel(duplicatedHandle);
	}

	return newEnemy;
}