#pragma once
#include "PopupBase.h"
#include <memory>
#include <vector>
#include <map>

class ReinforcementCard;
enum class ReinforcementType;

class PopupPlayerReinforcement : public PopupBase
{
public:
	PopupPlayerReinforcement();
	~PopupPlayerReinforcement();

	/// <summary>
	/// 初期化
	/// </summary>
	void Init() override;

	/// <summary>
	/// 内部変数の更新
	/// </summary>
	void Update() override;

	/// <summary>
	/// 描画全般
	/// </summary>
	void Draw() override;

	/// <summary>
	/// 終了処理
	/// </summary>
	void Final() override;

	/// <summary>
	/// 処理が全て完了したか
	/// </summary>
	/// <returns></returns>
	bool IsProcessingCompleted() override;

private:

	bool _isProcessingCompleted;

	int _playerSelectSerialNumber;	// プレイヤーが選択しているカード番号
	int _playerSelectCursorHandle;

	// タイプとカードのポインタを管理
	std::vector<std::pair<ReinforcementType, std::shared_ptr<ReinforcementCard>>> _cards;		// 表示を行うカード

	int _headingFontHandle;	// 見出し文字ハンドル
	int _enterFontHandle;

};

