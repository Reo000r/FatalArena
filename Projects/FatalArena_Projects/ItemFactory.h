#pragma once
#include "Vector3.h"
#include "PlayerBuffManager.h"

#include <memory>
#include <unordered_map>

class ItemBase;
class Physics;

/// <summary>
/// 指定されたアイテムの生成やモデルハンドルを管理している
/// </summary>
class ItemFactory final {
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
	static std::shared_ptr<ItemBase> CreateAndRegister(
		BuffType type,
		const Vector3& position,
		std::weak_ptr<PlayerBuffManager> manager,
		std::weak_ptr<Physics> physics
	);

	static float GetStrengthMulAmount();

private:
	// モデルハンドルを管理するためのコンテナ
	// キー:敵の種類, 値:モデルハンドル
	static std::unordered_map<BuffType, int> _modelHandles;
};

