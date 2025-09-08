#include "PlayerBuffGaugeDrawer.h"
#include "Vector2.h"
#include "Statistics.h"

#include <DxLib.h>
#include <string>

namespace {
	constexpr double kBaseScale = 1.0;
	constexpr double kGaugeScale = 1.5 * 4.0 * kBaseScale;
	constexpr double kIconScale = 1.0 * kBaseScale;
	constexpr int kGaugeAlpha = static_cast<int>(255 * 0.99f);
	constexpr int kDrawOffset = static_cast<int>(128 * kBaseScale);

	const Position2 kDrawBasePos = Position2(
		Statistics::kScreenCenterWidth * 0.75f,
		Statistics::kScreenCenterHeight * 0.2f);

	// アイコン画像のパス
	const std::unordered_map<BuffType, std::wstring> kIconPaths = {
		{ BuffType::Heal, L"data/icon/IconBuffHeal.png" },
		{ BuffType::Strength, L"data/icon/IconBuffStrength.png" },
		{ BuffType::ScoreBoost, L"data/icon/IconBuffScoreBoost.png" },
	};
	// ゲージ画像のパス
	const std::unordered_map<BuffType, std::wstring> kGaugePaths = {
		{ BuffType::Heal, L"data/icon/BuffGaugeRed.png" },
		{ BuffType::Strength, L"data/icon/BuffGaugeBlue.png" },
		{ BuffType::ScoreBoost, L"data/icon/BuffGaugeGreen.png" },
	};
}

PlayerBuffGaugeDrawer::PlayerBuffGaugeDrawer()
{
}

PlayerBuffGaugeDrawer::~PlayerBuffGaugeDrawer()
{
	for (const auto& pair : _buffIconHandles) {
		DeleteGraph(pair.second);
	}
	_buffIconHandles.clear();
	for (const auto& pair : _gaugeGraphHandles) {
		DeleteGraph(pair.second);
	}
	_gaugeGraphHandles.clear();
}

void PlayerBuffGaugeDrawer::Init(std::weak_ptr<PlayerBuffManager> manager)
{
	_manager = manager;

	// アイコン画像を読み込む
	for (const auto& pair : kIconPaths) {
		_buffIconHandles[pair.first] = LoadGraph(pair.second.c_str());
	}
	// ゲージ画像を読み込む
	for (const auto& pair : kGaugePaths) {
		_gaugeGraphHandles[pair.first] = LoadGraph(pair.second.c_str());
	}
}

void PlayerBuffGaugeDrawer::Draw()
{
	auto manager = _manager.lock();
	if (!manager) return;

	const auto& buffs = manager->GetBuffs();
	int drawCount = 0;

	for (const auto& buff : buffs) {
		if (!buff.isActive) continue;
		// 描画位置を計算
		int drawX = kDrawBasePos.x + drawCount * kDrawOffset;
		int drawY = kDrawBasePos.y;

		// ゲージの割合を計算
		double percent = 0.0;
		const int maxFrame = buff.maxActiveFrame;
		percent = static_cast<double>(buff.activeFrame) / maxFrame * 100.0;

		// ゲージを描画
		auto gaugeIt = _gaugeGraphHandles.find(buff.type);
		if (gaugeIt != _gaugeGraphHandles.end()) {
			SetDrawBlendMode(DX_BLENDMODE_MULA, kGaugeAlpha);
			DrawCircleGauge(drawX, drawY, percent, gaugeIt->second,
				0.0, kGaugeScale, false, false);
			// BlendModeを使った後はNOBLENDにしておくことを忘れず
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			
		}

		// アイコンを描画
		auto iconIt = _buffIconHandles.find(buff.type);
		if (iconIt != _buffIconHandles.end()) {
			int handle = iconIt->second;
			int w, h;
			GetGraphSize(handle, &w, &h);
			DrawRectRotaGraph(
				drawX, drawY,
				0,0,
				w,h,
				kIconScale, 0.0,
				handle, true,
				false, false);
		}
		drawCount++;
	}
}
