#include "GameManager.h"
#include "Player.h"
#include "WaveManager.h"
#include <algorithm>
#include <cassert>

GameManager::~GameManager()
{
}

GameManager& GameManager::GetInstance()
{
	// 初実行時にメモリ確保
	static GameManager manager;
	return manager;
}

void GameManager::Init(std::weak_ptr<Player> player, std::weak_ptr<WaveManager> waveManager)
{
	_player = player;
	_waveManager = waveManager;
	_enemyDefeatScore = 0;
	_clearTime = 0.0f;
	_timeBonusScore = 0;
}

void GameManager::Update()
{
	// 処理なし
}

void GameManager::ChangeFirstPlayState()
{
	// 呼ばれたら初回プレイ状態でなくなったとする
	_isFirstPlay = false;
}

void GameManager::AddEnemyDefeatScore(int score)
{
	if (score > 0) {
		_enemyDefeatScore += (int)(score * kDefeatScoreMul);
	}
}

void GameManager::UpdateClearTime()
{
	_clearTime += 1.0f / 60.0f;
}

int GameManager::GetCurrentWaveIndex() const
{
	auto waveManager = _waveManager.lock();
	if (waveManager) {
		// WaveManagerに現在のウェーブ数を返すgetterが必要
		return waveManager->GetCurrentWaveIndex();
	}
	return 0;
}

int GameManager::GetTotalWaves() const
{
	auto waveManager = _waveManager.lock();
	if (waveManager) {
		return waveManager->GetTotalWaveCount();
	}
	return 0;
}

bool GameManager::IsPlayerAlive() const
{
	auto player = _player.lock();
	if (player) {
		return player->IsAlive();
	}
	return false;
}

void GameManager::CalculateTimeBonus()
{
	// タイムが速いほど1.0、遅いほど0.0に近づく割合を計算
	// (現在のタイム - 最小ボーナスタイム) / (最大ボーナスタイム - 最小ボーナスタイム)
	// 分母が (速いタイム - 遅いタイム)なので、結果がマイナスになるため
	// -1をかけて反転させる
	float ratio = (_clearTime - kMinBonusTime) / (kMaxBonusTime - kMinBonusTime);

	// 割合を 0.0f - 1.0f の範囲に収める
	ratio = std::clamp(ratio, 0.0f, 1.0f);

	// 割合に応じてボーナススコアを線形補間で算出
	_timeBonusScore = static_cast<int>(kMinTimeBonusScore + (kMaxTimeBonusScore - kMinTimeBonusScore) * ratio);
	_timeBonusScore = (int)(_timeBonusScore * kTimeScoreMul);
}

void GameManager::SetResultScreenHandle(int handle)
{
	if (handle < 0) {
		assert(false && "不正なハンドル");
		return;
	}
	
	if (_resultScreenHandle != -1) {
		ReleaseResultScreen();
	}

	_resultScreenHandle = handle;
}

int GameManager::GetResultScreenHandle() const
{
	return 0;
}

void GameManager::ReleaseResultScreen()
{
}
