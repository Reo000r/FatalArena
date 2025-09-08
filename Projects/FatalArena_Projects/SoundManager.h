#pragma once
#include <unordered_map>

enum class SEType {
	Enter1,
	Enter2,
	Enter3,
	Swing1,
	Swing2,
	Attack1,
	Attack2,
	PlayerReact,
	PlayerDeath,
	ItemHeal,
	ItemStrength,
	ItemScoreBoost,
	WaveStart,
	SpawnEnemy,
	EnemyAttack,
	EnemyDeath,
	PlayerReinforcement,
	typeNum,
};

enum class BGMType {
	Title,
	GamePlay,
	Result,
	typeNum,
};

/// <summary>
/// BGMや効果音を管理するシングルトンクラス
/// </summary>
class SoundManager
{
private:
	SoundManager() :
		_seList(),
		_bgmList()
	{
	}
	SoundManager(const SoundManager&) = delete;
	void operator=(const SoundManager&) = delete;

public:
	static SoundManager& GetInstance();

	/// <summary>
	/// 必要なサウンドをすべて読み込む
	/// </summary>
	void LoadResources();

	/// <summary>
	/// 読み込んだサウンドをすべて解放する
	/// </summary>
	void ReleaseResources();

	void PlaySoundType(SEType type);
	void PlaySoundType(BGMType type, bool isLoop = true, bool isPlayFromStart = true);

private:

	// SE種別、ハンドルを管理
	std::unordered_map<SEType, int> _seList;

	// BGM種別、ハンドルを管理
	std::unordered_map<BGMType, int> _bgmList;
};

