#include "StatusUI.h"
#include "Player.h"
#include "WaveManager.h"
#include "EnemyManager.h"
#include "EnemyBase.h"
#include "GameManager.h"
#include "Statistics.h"
#include <DxLib.h>
#include <string>

namespace {
	// プレイヤーHPバー
	const int kPlayerHpBarPosX = Statistics::kScreenWidth * 0.02f;
	constexpr int kPlayerHpBarPosY = Statistics::kScreenHeight * 0.05f;
	constexpr int kPlayerHpBarWidth = Statistics::kScreenWidth * 0.3f;
	constexpr int kPlayerHpBarHeight = Statistics::kScreenHeight * 0.05f;

	const unsigned int kPlayerHpBarBackColor = GetColor(50, 50, 50);
	const unsigned int kPlayerHpBarFrontColor = GetColor(0, 200, 100);
	const unsigned int kPlayerHpBarFrameColor = GetColor(255, 255, 255);
	
	// プレイヤースタミナバー
	const int kPlayerStaminaBarPosX = Statistics::kScreenWidth * 0.02f;
	constexpr int kPlayerStaminaBarPosY = kPlayerHpBarPosY + Statistics::kScreenHeight * (0.05f + 0.01f);
	constexpr int kPlayerStaminaBarWidth = Statistics::kScreenWidth * 0.3f;
	constexpr int kPlayerStaminaBarHeight = Statistics::kScreenHeight * 0.05f;

	const unsigned int kPlayerStaminaBarBackColor = GetColor(50, 50, 50);
	const unsigned int kPlayerStaminaBarFrontColor = GetColor(180, 180, 0);
	const unsigned int kPlayerStaminaBarFrameColor = GetColor(255, 255, 255);


	// 敵HPバー
	constexpr int kEnemyHpBarWidth = Statistics::kScreenWidth * 0.1f;
	constexpr int kEnemyHpBarHeight = Statistics::kScreenHeight * 0.02f;
	const unsigned int kEnemyHpBarBackColor = GetColor(50, 50, 50);
	const unsigned int kEnemyHpBarFrontColor = GetColor(220, 50, 50);
	const unsigned int kEnemyHpBarFrameColor = GetColor(255, 255, 255);
	const Position3 kEnemyBarOffset = Vector3(0, 600, 0);


	// スコア表示
	const int kScoreFontSize = Statistics::kScreenWidth * 0.038f;
	const int kScorePosX = Statistics::kScreenWidth - kScoreFontSize * 2;	// 描画位置
	const int kScorePosY = kScoreFontSize * 0.75f;
	const unsigned int kScoreColor = GetColor(255, 255, 255);
	const std::wstring kScoreFontName = L"Impact"; // フォント名
}

StatusUI::StatusUI():
	_scoreFontHandle(-1)
{
}

StatusUI::~StatusUI()
{
	// フォントハンドルが有効なら削除
	if (_scoreFontHandle != -1) {
		DeleteFontToHandle(_scoreFontHandle);
	}
}

void StatusUI::Init(std::weak_ptr<Player> player, std::weak_ptr<WaveManager> waveManager, std::weak_ptr<EnemyManager> enemyManager)
{
	_player = player;
	_waveManager = waveManager;
	_enemyManager = enemyManager;

	_scoreFontHandle = CreateFontToHandle(
		kScoreFontName.c_str(),
		kScoreFontSize,
		-1,
		DX_FONTTYPE_ANTIALIASING_EDGE);
}

void StatusUI::Update()
{
	// hitpoint情報を更新
}

void StatusUI::Draw()
{
	DrawScore();

	DrawPlayerHp();
	DrawPlayerStamina();

	// 敵HP描画
	if (auto enemyManager = _enemyManager.lock()) {
		const auto& enemies = enemyManager->GetEnemies();
		for (const auto& enemy : enemies) {
			if (enemy) {
				DrawEnemyHp(enemy);
			}
		}
	}
}

void StatusUI::DrawPlayerHp()
{
	if (auto player = _player.lock()) {
		float maxHp = player->GetMaxHitPoint();
		if (maxHp <= 0.0f) return;
		float playerHitPoint = _player.lock()->GetHitPoint();
		//if (playerHitPoint <= 0.0f) return;

		// HPの割合を計算
		float hpRatio = playerHitPoint / maxHp;

		// 背景バー
		DrawBox(kPlayerHpBarPosX, kPlayerHpBarPosY,
			kPlayerHpBarPosX + kPlayerHpBarWidth, kPlayerHpBarPosY + kPlayerHpBarHeight,
			kPlayerHpBarBackColor, true);

		// 前景バー
		if (playerHitPoint > 0.0f) {
			DrawBox(kPlayerHpBarPosX, kPlayerHpBarPosY,
				kPlayerHpBarPosX + static_cast<int>(kPlayerHpBarWidth * hpRatio),
				kPlayerHpBarPosY + kPlayerHpBarHeight,
				kPlayerHpBarFrontColor, true);
		}
		
		// 100刻みで縦線を描画
		for (int i = 100; i < maxHp; i += 100) {
			int lineX = kPlayerHpBarPosX + static_cast<int>(kPlayerHpBarWidth * (i / maxHp));
			DrawLine(lineX, kPlayerHpBarPosY, lineX, kPlayerHpBarPosY + kPlayerHpBarHeight,
				kPlayerHpBarFrameColor);
		}

		// 枠線
		DrawBox(kPlayerHpBarPosX, kPlayerHpBarPosY,
			kPlayerHpBarPosX + kPlayerHpBarWidth, kPlayerHpBarPosY + kPlayerHpBarHeight,
			kPlayerHpBarFrameColor, false);
	}
}

void StatusUI::DrawPlayerStamina()
{
	if (auto player = _player.lock()) {
		float maxStamina = player->GetMaxStamina();
		if (maxStamina > 0.0f) {
			float stamina = _player.lock()->GetStamina();
			// 割合を計算
			float ratio = stamina / maxStamina;

			// 背景バー
			DrawBox(kPlayerStaminaBarPosX, kPlayerStaminaBarPosY,
				kPlayerStaminaBarPosX + kPlayerStaminaBarWidth, kPlayerStaminaBarPosY + kPlayerStaminaBarHeight,
				kPlayerStaminaBarBackColor, true);

			// 前景バー
			if (stamina > 0.0f) {
				DrawBox(kPlayerStaminaBarPosX, kPlayerStaminaBarPosY,
					kPlayerStaminaBarPosX + static_cast<int>(kPlayerStaminaBarWidth * ratio),
					kPlayerStaminaBarPosY + kPlayerStaminaBarHeight,
					kPlayerStaminaBarFrontColor, true);
			}
			
			// 10刻みで縦線を描画
			for (int i = 10; i < maxStamina; i += 10) {
				int lineX = kPlayerStaminaBarPosX + static_cast<int>(kPlayerStaminaBarWidth * (i / maxStamina));
				DrawLine(lineX, kPlayerStaminaBarPosY, lineX, kPlayerStaminaBarPosY + kPlayerStaminaBarHeight,
					kPlayerStaminaBarFrameColor);
			}

			// 枠線
			DrawBox(kPlayerStaminaBarPosX, kPlayerStaminaBarPosY,
				kPlayerStaminaBarPosX + kPlayerStaminaBarWidth, kPlayerStaminaBarPosY + kPlayerStaminaBarHeight,
				kPlayerStaminaBarFrameColor, false);
		}
	}
}

void StatusUI::DrawEnemyHp(std::shared_ptr<EnemyBase> enemy)
{
	// 敵のHPと最大HPを取得
	float currentHp = enemy->GetHitPoint();
	float maxHp = enemy->GetMaxHitPoint();
	// HPが0以下、または最大HPが0以下の敵は描画しない
	if (currentHp <= 0.0f || maxHp <= 0.0f) return;

	// 敵の頭上座標を計算
	Vector3 enemyPos = enemy->GetPos();
	Vector3 barWorldPos = enemyPos + kEnemyBarOffset;

	// 3D座標を2Dスクリーン座標に変換
	Position3 screenPos = ConvWorldPosToScreenPos(barWorldPos);

	// 画面外の場合は描画しない (Z座標が1.0fより大きいと画面奥)
	if (screenPos.z > 1.0f) return;


	// HPバーの中心がscreenPosに来るように、描画開始位置を計算
	int barStartX = static_cast<int>(screenPos.x) - static_cast<int>(kEnemyHpBarWidth * 0.5f);
	int barStartY = static_cast<int>(screenPos.y) - static_cast<int>(kEnemyHpBarHeight * 0.5f);

	// HPの割合を計算
	float hpRatio = currentHp / maxHp;

	// 背景バー
	DrawBox(barStartX, barStartY,
		barStartX + kEnemyHpBarWidth, barStartY + kEnemyHpBarHeight,
		kEnemyHpBarBackColor, true);

	// 前景バー
	DrawBox(barStartX, barStartY,
		barStartX + static_cast<int>(kEnemyHpBarWidth * hpRatio), barStartY + kEnemyHpBarHeight,
		kEnemyHpBarFrontColor, true);

	// 50刻みで縦線を描画
	for (int i = 50; i < maxHp; i += 50) {
		int lineX = barStartX + static_cast<int>(kEnemyHpBarWidth * (i / maxHp));
		DrawLine(lineX, barStartY,
			lineX, barStartY + kEnemyHpBarHeight,
			kEnemyHpBarFrameColor);
	}

	// 枠線
	DrawBox(barStartX, barStartY,
		barStartX + kEnemyHpBarWidth, barStartY + kEnemyHpBarHeight,
		kEnemyHpBarFrameColor, false);
}

void StatusUI::DrawScore()
{
	// GameManagerから敵撃破スコアを取得
	int score = GameManager::GetInstance().GetEnemyDefeatScore();

	// スコアの文字列を作成
	std::wstring scoreText = L"Score : ";

	// 描画するX座標を計算 (右揃え)
	// まず"SCORE: "部分の幅を計算
	int textWidth = GetDrawStringWidthToHandle(scoreText.c_str(), static_cast<int>(scoreText.length()),
		_scoreFontHandle);
	// 次に数字部分の幅を計算
	int numWidth = GetDrawStringWidthToHandle(L"0000000", 5, _scoreFontHandle);

	int totalWidth = textWidth + numWidth;
	int drawX = kScorePosX - totalWidth;

	// 文字を描画
	DrawStringToHandle(drawX, kScorePosY, scoreText.c_str(), kScoreColor, _scoreFontHandle);

	// スコアを0埋めで描画
	DrawFormatStringToHandle(drawX + textWidth, 
		kScorePosY, kScoreColor, _scoreFontHandle, 
		L"%07d",score);
}
