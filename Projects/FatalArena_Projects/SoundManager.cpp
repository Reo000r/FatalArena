#include "SoundManager.h"
#include <string>
#include <cassert>
#include <DxLib.h>

namespace {
	// サウンドファイルのパスをここで管理
	const std::unordered_map<SEType, std::wstring> kSEPaths = {
		{ SEType::Enter1,		L"data/sound/se/SEEnter1.mp3" },
		{ SEType::Enter2,		L"data/sound/se/SEEnter2.mp3" },
		{ SEType::Enter3,		L"data/sound/se/SEEnter3.mp3" },
		{ SEType::Swing1,		L"data/sound/se/SESwing1.mp3" },
		{ SEType::Swing2,		L"data/sound/se/SESwing2.mp3" },
		{ SEType::Attack1,		L"data/sound/se/SEAttack1.mp3" },
		{ SEType::Attack2,		L"data/sound/se/SEAttack2.mp3" },
		{ SEType::PlayerReact,	L"data/sound/se/SEPlayerReact.mp3" },
		{ SEType::PlayerDeath,	L"data/sound/se/SEPlayerDeath.mp3" },
		{ SEType::ItemHeal,		L"data/sound/se/SEItemHeal.mp3" },
		{ SEType::ItemStrength,	L"data/sound/se/SEItemStrength.mp3" },
		{ SEType::ItemScoreBoost,	L"data/sound/se/SEItemScoreBoost.mp3" },
		{ SEType::WaveStart,	L"data/sound/se/SEWaveStart.mp3" },
		{ SEType::SpawnEnemy,	L"data/sound/se/SESpawnEnemy.mp3" },
		{ SEType::EnemyAttack,	L"data/sound/se/SEEnemyAttack.mp3" },
		{ SEType::EnemyDeath,	L"data/sound/se/SEEnemyDeath.mp3" },
		{ SEType::PlayerReinforcement,	L"data/sound/se/SEPlayerReinforcement.mp3" },
	};
	const std::unordered_map<BGMType, std::wstring> kBGMPaths = {
		{ BGMType::Title,		L"data/sound/bgm/BGMTitle.mp3" },
		{ BGMType::GamePlay, L"data/sound/bgm/BGMGamePlay.mp3" },
		{ BGMType::Result,	L"data/sound/bgm/BGMResult.mp3" },
	};

	constexpr float kMasterVolumeRatio = 0.7f;
	constexpr int kSEVolume = static_cast<int>(255 * 1.0f * kMasterVolumeRatio);
	constexpr int kBGMVolume = static_cast<int>(255 * 0.6f * kMasterVolumeRatio);
}

SoundManager& SoundManager::GetInstance()
{
	// 初実行時にメモリ確保
	static SoundManager manager;
	return manager;
}

void SoundManager::LoadResources()
{
	// 全てのサウンドを読み込み、ハンドルを保存する
	for (const auto& pair : kSEPaths) {
		const SEType& type = pair.first;
		const std::wstring& path = pair.second;
		int handle = LoadSoundMem(path.c_str());
		assert(handle != -1 && "サウンドの読み込みに失敗");
		_seList[type] = handle;
		ChangeVolumeSoundMem(kSEVolume, handle);
	}
	for (const auto& pair : kBGMPaths) {
		const BGMType& type = pair.first;
		const std::wstring& path = pair.second;
		int handle = LoadSoundMem(path.c_str());
		assert(handle != -1 && "サウンドの読み込みに失敗");
		_bgmList[type] = handle;
		ChangeVolumeSoundMem(kBGMVolume, handle);
	}
}

void SoundManager::ReleaseResources()
{
	// 保存されている全てのサウンドを解放する
	for (const auto& pair : _seList) {
		DeleteSoundMem(pair.second);
	}
	_seList.clear();
	for (const auto& pair : _bgmList) {
		DeleteSoundMem(pair.second);
	}
	_bgmList.clear();
}

void SoundManager::PlaySoundType(SEType type)
{
	// 複製元のサウンドが存在するかチェック
	auto it = _seList.find(type);
	assert(it != _seList.end() && "要求されたタイプのサウンドが読み込まれていない");

	// 効果音再生
	PlaySoundMem(it->second, DX_PLAYTYPE_BACK, true);
}

void SoundManager::PlaySoundType(BGMType type, bool isLoop, bool isPlayFromStart)
{
	// 複製元のサウンドが存在するかチェック
	auto it = _bgmList.find(type);
	assert(it != _bgmList.end() && "要求されたタイプのサウンドが読み込まれていない");

	for (auto& pair : _bgmList) {
		// 同じBGMかつ途中からの再生を許可している場合はcontinue
		if (!isPlayFromStart && type == pair.first) continue;
		// 既存のBGMが鳴っていたら停止
		if (CheckSoundMem(pair.second) == 1) {
			StopSoundMem(pair.second);
		}
	}

	// 指定された曲がなっていたかつ
	// 最初からの再生を希望されていないならreturn
	if (CheckSoundMem(_bgmList[type]) == 1 &&
		!isPlayFromStart) return;

	// BGM再生
	int playType = DX_PLAYTYPE_LOOP;
	if (!isLoop) playType = DX_PLAYTYPE_BACK;
	PlaySoundMem(it->second, playType, true);
}
