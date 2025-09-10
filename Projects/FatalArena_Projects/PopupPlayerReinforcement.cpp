#include "PopupPlayerReinforcement.h"
#include "ReinforcementCard.h"
#include "PlayerReinforcementManager.h"
#include "Statistics.h"
#include "Input.h"
#include "SoundManager.h"

#include <DxLib.h>
#include <cassert>
#include <string>

namespace {

	const unsigned int kTextColor = GetColor(255, 255, 255);
	const std::wstring kFontName = Statistics::kDefaultFontName;
	constexpr int kHeadingFontSize = 92;		// 見出し文字サイズ
	constexpr int kEnterFontSize = 64;		// 案内文字サイズ
	constexpr int kFontThickness = 3;
	//const std::wstring kHeadingText = L"プレイヤー強化";
	//const std::wstring kPadEnterText = L"強化内容を選べ！\n Aボタンで決定";
	//const std::wstring kKeybdEnterText = L" 強化内容を選べ！\nエンターキーで決定";
	const std::wstring kHeadingText = L"Reinforcement   Phase";
	const std::wstring kPadEnterText = L"Choose the Power You Want!\n          Press A to Confirm";
	const std::wstring kKeybdEnterText = L"Choose the Power You Want!\n      Press Enter to Confirm";

	// 描画レイアウト用の定数
	constexpr int kHeadingTextY = Statistics::kScreenHeight * 0.1f;						// 見出し文字のY座標
	constexpr int kEnterYOffset = kHeadingTextY + Statistics::kScreenHeight * 0.7f;		// 見出しと案内の距離

	// カード画像のパス
	const std::vector<std::wstring> kCardPaths = {
		{ L"data/graph/card/PlayerReinforcementCardRed.png" },
		{ L"data/graph/card/PlayerReinforcementCardGreen.png" },
		{ L"data/graph/card/PlayerReinforcementCardBlue.png" },
		{ L"data/graph/card/PlayerReinforcementCardYellow.png" },
		{ L"data/graph/card/PlayerReinforcementCardPurple.png" },
	};
	// アイコン画像のパス
	const std::vector<std::wstring> kCardIconPaths = {
		{ L"data/icon/IconHealth.png" },
		{ L"data/icon/IconBoots.png" },
		{ L"data/icon/IconStrength.png" },
		{ L"data/icon/IconDefence.png" },
		{ L"data/icon/IconLuck.png" },
	};

	constexpr int kMinPresentationCardAmount = 3;	// 提示するカード枚数の最低値
	constexpr int kMaxPresentationCardAmount = 3;	// 提示するカード枚数の最高値
	constexpr int kPresentationCardAmountDifference = kMaxPresentationCardAmount - kMinPresentationCardAmount;	// 提示するカード枚数の差

	constexpr float kCardScaleMulOffset = 3.0f;				// 描画カード拡大倍率
	constexpr float kIconScaleMulOffset = 2.0f;				// 描画アイコン拡大倍率

	// 強化内容
	constexpr float kHealthReinforceAmount = 0.2f;
	constexpr float kStaminaReinforceAmount = 0.2f;
	constexpr float kStrengthReinforceAmount = 0.15f;
}

PopupPlayerReinforcement::PopupPlayerReinforcement() :
	_isProcessingCompleted(false),
	_playerSelectSerialNumber(0),
	_playerSelectCursorHandle(-1),
	_cards()
{
	_headingFontHandle = CreateFontToHandle(kFontName.c_str(), kHeadingFontSize, kFontThickness,
		DX_FONTTYPE_ANTIALIASING_EDGE);
	assert(_headingFontHandle >= 0 && "フォントの作成に失敗");
	_enterFontHandle = CreateFontToHandle(kFontName.c_str(), kEnterFontSize, kFontThickness,
		DX_FONTTYPE_ANTIALIASING_EDGE);
	assert(_enterFontHandle >= 0 && "フォントの作成に失敗");

	_playerSelectCursorHandle = LoadGraph(kCardIconPaths[4].c_str());
	assert(_playerSelectCursorHandle >= 0 && "不正なハンドル");
}

PopupPlayerReinforcement::~PopupPlayerReinforcement()
{
	if (_playerSelectCursorHandle != -1) {
		DeleteGraph(_playerSelectCursorHandle);
	}
}

void PopupPlayerReinforcement::Init()
{
	// カードの生成
	
	// カード枚数の決定
	const int generateCardAmount = 
		kMinPresentationCardAmount + GetRand(kPresentationCardAmountDifference);

	for (int i = 0; i < generateCardAmount; i++) {
		// カード情報の決定
		int cardSerialNum = i;//GetRand(static_cast<int>(CardType::TypeNum));
		if (cardSerialNum >= static_cast<int>(CardType::TypeNum)) {
			assert(false && "不正なカードタイプ");
			cardSerialNum = 0;
		}
		int cardHandle = LoadGraph(kCardPaths[cardSerialNum].c_str());
		int iconHandle = LoadGraph(kCardIconPaths[cardSerialNum].c_str());
		
		// 画面の幅と端からの余白と描画枚数に考慮して位置決定を行う
		Position3 centerPos;
		// カード同士の間隔
		int cardWidth = Statistics::kScreenWidth / generateCardAmount;
		centerPos.x = cardWidth * i + cardWidth * 0.5f;
		centerPos.y = Statistics::kScreenCenterHeight;
		centerPos.z = 0.0f;

		// カード生成
		std::shared_ptr<ReinforcementCard> card = 
			std::make_shared<ReinforcementCard>();
		card->Init(cardHandle, iconHandle, centerPos, kCardScaleMulOffset, kIconScaleMulOffset);

		ReinforcementType type = static_cast<ReinforcementType>(cardSerialNum);
		// 登録
		std::pair<ReinforcementType, std::shared_ptr<ReinforcementCard>> pair;
		pair.first = type;
		pair.second = card;
		_cards.emplace_back(pair);
	}

	// 中央にカーソルを置く
	_playerSelectSerialNumber = (int)(generateCardAmount * 0.5f);

	SoundManager::GetInstance().PlaySoundType(SEType::Enter3);
}

void PopupPlayerReinforcement::Update()
{
	if (_isProcessingCompleted) return;

	Input& input = Input::GetInstance();

	// 決定した場合
	if (input.IsTrigger("Gameplay:Enter")) {
		// 強化内容データを作成
		ReinforcementData data;
		data.type = _cards[_playerSelectSerialNumber].first;
		switch (data.type) {
		case ReinforcementType::MaxHealth:
			data.reinforceAmount = kHealthReinforceAmount;
			break;
		case ReinforcementType::Stamina:
			data.reinforceAmount = kStaminaReinforceAmount;
			break;
		case ReinforcementType::Strength:
			data.reinforceAmount = kStrengthReinforceAmount;
			break;
		default:
			assert(false && "不明なカードタイプ");
			break;
		}

		// 強化内容を伝える
		PlayerReinforcementManager::AttachReinforcementData(data);
		// 処理終了
		_isProcessingCompleted = true;
		SoundManager::GetInstance().PlaySoundType(SEType::PlayerReinforcement);
		return;
	}

	// 選択を行わせる

	// カーソル移動処理
	if (input.IsTrigger("Gameplay:Left")) {
		_playerSelectSerialNumber--;
	}
	if (input.IsTrigger("Gameplay:Right")) {
		_playerSelectSerialNumber++;
	}

	// オーバーしていた場合はループさせる
	if (_playerSelectSerialNumber < 0) {
		_playerSelectSerialNumber = (int)_cards.size() - 1;
	}
	if (_playerSelectSerialNumber >= (int)_cards.size()) {
		_playerSelectSerialNumber = 0;
	}

	// カード更新
	for (auto& card : _cards) {
		card.second->Update();
	}
}

void PopupPlayerReinforcement::Draw()
{
	// カード描画
	for (auto& card : _cards) {
		card.second->Draw();
	}

	// 選択カーソル描画
	int cursorWidth, cursorHeight;
	GetGraphSize(_playerSelectCursorHandle, &cursorWidth, &cursorHeight);
	Position3 cursorPos = _cards[_playerSelectSerialNumber].second->GetCenterPos();
	cursorPos.y -= Statistics::kScreenCenterHeight * 0.7f;
	DrawRectRotaGraph((int)cursorPos.x, (int)cursorPos.y,
		0,0,
		cursorWidth, cursorHeight,
		2.0, 0.0,
		_playerSelectCursorHandle,
		true,
		false, false);



	// 見出し、案内文字の描画

	// 文字の中央までの長さをはかり
	// 描画開始位置を求め描画する
	int headingTextWidth = GetDrawStringWidthToHandle(
		kHeadingText.c_str(),
		(int)kHeadingText.length(),
		_headingFontHandle);
	int headingDrawX = (int)((Statistics::kScreenWidth - headingTextWidth) * 0.5f);
	DrawStringToHandle(
		headingDrawX, kHeadingTextY,
		kHeadingText.c_str(), kTextColor,
		_headingFontHandle);

	// 最後の入力に応じて文字を変える
	std::wstring drawString = kPadEnterText;
	if (Input::GetInstance().GetLastInputType() == Input::PeripheralType::keybd) {
		drawString = kKeybdEnterText;
	}

	// 文字の中央までの長さをはかり
	// 描画開始位置を求め描画する
	int enterTextWidth = GetDrawStringWidthToHandle(
		drawString.c_str(),
		(int)drawString.length(),
		_enterFontHandle);
	int enterDrawX = (int)((Statistics::kScreenWidth - enterTextWidth) * 0.5f);
	DrawStringToHandle(
		enterDrawX, kEnterYOffset,
		drawString.c_str(), kTextColor,
		_enterFontHandle);
}

void PopupPlayerReinforcement::Final()
{
	// 処理なし
}

bool PopupPlayerReinforcement::IsProcessingCompleted()
{
	return _isProcessingCompleted;
}
