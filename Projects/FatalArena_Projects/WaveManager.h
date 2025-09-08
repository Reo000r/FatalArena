#pragma once
#include "WaveData.h"
#include <vector>
#include <memory>

class EnemyManager;
class ItemManager;
class WaveAnnouncer;

class WaveManager {
private:
	enum class State {
		Announcing,
		Spawning,
		InProgress,
		ReinforcementSelect,
		WaitingForCleanup,
		AllWavesComplete
	};

public:
	WaveManager();
	~WaveManager();

	void Init(std::weak_ptr<EnemyManager> enemyManager, 
		std::weak_ptr<ItemManager> itemManager, 
		std::weak_ptr<WaveAnnouncer> waveAnnouncer);
	void Update();

	void StartAnnounce();

	bool IsClear() const { return _state == State::AllWavesComplete; }
	int GetCurrentWaveIndex() const { return _currentWaveIndex; }
	int GetTotalWaveCount() const { return static_cast<int>(_waveSettings.size()); }
	bool CanReinforcement() const { return (_state == State::ReinforcementSelect); }

	/// <summary>
	/// 強化が終わったら呼ばれる
	/// </summary>
	void StartCleanup();

private:
	void InitWaveSettings();
	void CheckWaveCompletion();
	void TransitionToNextWave();

private:
	State _state;
	int _currentWaveIndex;
	int _waveTransitionFrameCount;
	std::vector<WaveData> _waveSettings;

	
	std::weak_ptr<EnemyManager> _enemyManager;
	std::weak_ptr<ItemManager> _itemManager;
	std::weak_ptr<WaveAnnouncer> _waveAnnouncer;
};