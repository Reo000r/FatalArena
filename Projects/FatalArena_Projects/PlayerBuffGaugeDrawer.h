#pragma once
#include "PlayerBuffManager.h"
#include <memory>
#include <unordered_map>

class PlayerBuffManager;

class PlayerBuffGaugeDrawer
{
public:
	PlayerBuffGaugeDrawer();
	/// <summary>
	/// 画像ハンドルの解放
	/// </summary>
	~PlayerBuffGaugeDrawer();

	/// <summary>
	/// 画像の読み込み
	/// </summary>
	/// <param name="manager"></param>
	void Init(std::weak_ptr<PlayerBuffManager> manager);
	/// <summary>
	/// メンバ変数をもとにゲージを描画
	/// </summary>
	void Draw();

private:

	std::weak_ptr<PlayerBuffManager> _manager;
	// バフアイコンのハンドル
	std::unordered_map<BuffType, int> _buffIconHandles;
	// ゲージ画像のハンドル
	std::unordered_map<BuffType, int> _gaugeGraphHandles;
};

