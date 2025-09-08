#pragma once
#include <vector>
#include <memory>

// 前方宣言
class EnemyBase;
class Player;
class Physics;
struct SpawnInfo;
struct WaveData;

class EnemyManager
{
public:
	EnemyManager();
	~EnemyManager();

	/// <summary>
	/// 初期化
	/// </summary>
	void Init(std::weak_ptr<Player> player, std::weak_ptr<Physics> physics);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 指定された情報に基づいて敵を生成する
	/// </summary>
	/// <param name="spawnInfoList">生成する敵の情報リスト</param>
	void SpawnEnemies(const std::vector<SpawnInfo>& spawnInfoList);

	/// <summary>
	/// 管理している全ての敵が倒されたか
	/// </summary>
	/// <returns>true:全滅 false:生存あり</returns>
	bool AreAllEnemiesDefeated() const;

	/// <summary>
	/// 管理している敵のリストを取得する
	/// </summary>
	/// <returns>敵のリスト</returns>
	const std::vector<std::shared_ptr<EnemyBase>>& GetEnemies() const;

private:
	/// <summary>
	/// 倒された(死亡が完了した)敵をリストから削除する
	/// </summary>
	void CleanupDefeatedEnemies();

private:
	std::vector<std::shared_ptr<EnemyBase>> _enemies;

	// 敵を生成する際に必要な情報
	std::weak_ptr<Player> _player;
	std::weak_ptr<Physics> _physics;
};
