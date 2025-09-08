#pragma once

#include <memory>
#include <unordered_map>
#include "Vector3.h"

class EnemyBase;
class Player;
class Physics;

enum class EnemyType {
	Normal,
};

class EnemyFactory final {
public:
	/// <summary>
	/// 必要なモデルをすべて読み込む
	/// </summary>
	static void LoadResources();

	/// <summary>
	/// 読み込んだモデルをすべて解放する
	/// </summary>
	static void ReleaseResources();

	/// <summary>
	/// 敵の種類に応じてインスタンスを生成し、
	/// 初期化とphysicsへの登録を行う
	/// </summary>
	static std::shared_ptr<EnemyBase> CreateAndRegister(
		EnemyType type,
		const Vector3& position,
		std::weak_ptr<Player> player,
		std::weak_ptr<Physics> physics
	);

private:
	// モデルハンドルを管理するためのコンテナ
	// キー:敵の種類, 値:モデルハンドル
	static std::unordered_map<EnemyType, int> _modelHandles;
};