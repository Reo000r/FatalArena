#include "WaveManager.h"
#include "EnemyFactory.h"
#include "EnemyBase.h"
#include "EnemyManager.h"
#include "ItemBase.h"
#include "ItemManager.h"
#include "WaveAnnouncer.h"
#include "Calculation.h"
#include "SoundManager.h"

#include <DxLib.h>
#include <cassert>

namespace {
	constexpr int kTotalWaves = 7;			// ウェーブ数
	constexpr int kBaseEnemyCount = 1;		// 初期出現敵数
	constexpr int kWavesPerIncrease = 2;	// このウェーブ数超えると敵が増える
	constexpr int kIncreaseAmount = 1;		// 敵増加量

	const Position3 kSpawnCenterPos = Vector3(0.0f, 0.0f, 700.0f);	// 敵出現中心位置
	constexpr float kSpawnRadius = 1200.0f;	// 敵出現半径

	// ウェーブ間のインターバル時間(秒)
	constexpr int kWaveTransitionIntervalFrame = (int)(60.0f * 0.5f);
}

WaveManager::WaveManager() :
	_state(State::Announcing),
	_currentWaveIndex(0),
	_waveTransitionFrameCount(0),
	_enemyManager(),
	_itemManager(),
	_waveAnnouncer()
{
}

WaveManager::~WaveManager() {}

void WaveManager::Init(std::weak_ptr<EnemyManager> enemyManager, 
	std::weak_ptr<ItemManager> itemManager, 
	std::weak_ptr<WaveAnnouncer> waveAnnouncer)
{
	_enemyManager = enemyManager;
	_itemManager = itemManager;
	_waveAnnouncer = waveAnnouncer;

	InitWaveSettings();
}

void WaveManager::Update()
{
	switch (_state) {
	case State::Announcing:
		if (_waveAnnouncer.lock()->IsFinished()) {
			_state = State::Spawning;
			SoundManager::GetInstance().PlaySoundType(SEType::SpawnEnemy);
		}
		break;
	case State::Spawning:
	{
		const auto& spawnInfo = _waveSettings[_currentWaveIndex].spawnGroups;
		_enemyManager.lock()->SpawnEnemies(spawnInfo);
		for (int i = 0; i < 2; ++i) {
			BuffType spawnType = static_cast<BuffType>(GetRand(static_cast<int>(BuffType::TypeNum)-1));
			_itemManager.lock()->SpawnItem(spawnType);
		}
		_state = State::InProgress;

	}
	break;
	case State::InProgress:
		CheckWaveCompletion();
		break;
	case State::ReinforcementSelect:
		// 処理なし
		break;
	case State::WaitingForCleanup:
		_waveTransitionFrameCount++;
		if (_waveTransitionFrameCount >= kWaveTransitionIntervalFrame) {
			TransitionToNextWave();
		}
		break;
	case State::AllWavesComplete:
		printf("");
		break;
	}
}

void WaveManager::StartAnnounce()
{
	_waveAnnouncer.lock()->Start(_currentWaveIndex + 1);
}

void WaveManager::CheckWaveCompletion()
{
	if (_enemyManager.lock()->AreAllEnemiesDefeated()) {
		// 最後のウェーブかどうかで分岐
		if (_currentWaveIndex < GetTotalWaveCount() - 1) {
			_state = State::ReinforcementSelect;
		}
		else {
			_state = State::WaitingForCleanup;
		}
		_waveTransitionFrameCount = 0;
	}
}

void WaveManager::TransitionToNextWave()
{
	_currentWaveIndex++;
	if (_currentWaveIndex < GetTotalWaveCount()) {
		_waveAnnouncer.lock()->Start(_currentWaveIndex + 1);
		_state = State::Announcing;
	}
	else {
		_state = State::AllWavesComplete;
	}
}

void WaveManager::StartCleanup()
{
	_state = State::WaitingForCleanup;
}

void WaveManager::InitWaveSettings()
{
	// 最初のウェーブで出現する敵の数
	int spawnAmount = kBaseEnemyCount;
	// kTotalWaves の数だけウェーブ設定を生成
	for (int i = 0; i < kTotalWaves; ++i) {
		WaveData wave;	// ウェーブデータ
		SpawnInfo info;	// 1グループ分の敵の出現情報を格納する変数
		info.type = EnemyType::Normal;	// 出現させる敵の種類設定
		// 特定のウェーブ数を超えるごとに、出現する敵の数を増やす
		if ((i + 1) % kWavesPerIncrease == 0) spawnAmount += kIncreaseAmount;
		info.count = spawnAmount;				// 出現数
		info.basePosition = kSpawnCenterPos;	// 生成中心位置
		info.spawnRadius = kSpawnRadius;		// この半径内にランダム配置

		// 設定登録
		wave.spawnGroups.push_back(info);
		_waveSettings.push_back(wave);
	}
}
