#include "ResultDisplay.h"
#include "Statistics.h"
#include "GameManager.h"
#include "ResultItemDrawer.h"
#include "Input.h"
#include <DxLib.h>
#include <cassert>
#include <string>

namespace {

    const unsigned int kTextColor = GetColor(255, 255, 255);
    const std::wstring kFontName = Statistics::kDefaultFontName;
    constexpr int kResultFontSize = 128;     // 上部文字用サイズ
    constexpr int kNextSceneFontSize = 96;   // 案内文字用サイズ
    constexpr int kFontThickness = 3;
    const std::wstring kResultText = L"RESULT";
    const std::wstring kScoreText = L"SCORE : ";
    const std::wstring kTimeText = L"TIME  : ";
    const std::wstring kTimeBonusText = L"TIME BONUS : ";
    const std::wstring kTotalScoreText = L"TOTAL SCORE : ";
    //const std::wstring kResultText = L"リザルト";
    //const std::wstring kScoreText = L"スコア : ";
    //const std::wstring kTimeText = L"タイム  : ";
    //const std::wstring kTimeBonusText = L"タイムボーナス : ";
    //const std::wstring kTotalScoreText = L"合計スコア : ";
    const std::wstring kPadNextSceneText = L"Press A to Restart\n   Press B to Title";
    const std::wstring kKeybdNextSceneText = L"Press Enter to Restart\n     Press Esc to Title";
    //const std::wstring kPadNextSceneText = L"Aボタンでリスタート\n   Bボタンでタイトル";
    //const std::wstring kKeybdNextSceneText = L"エンターキーでリスタート\n     エスケープキーでタイトル";
    

    // アニメーション用の定数
    constexpr int kSlideInDuration = 45;            // スライドインにかかるフレーム
    constexpr int kNextSceneFontThicknessIn = 40;	// 文字の出現期間(フレーム)
    constexpr int kNextSceneFontThicknessOut = 20;	// 文字の消滅期間(フレーム)
    constexpr int kNextSceneFontThicknessChange = 4;	// シーン切り替え中の文字の点滅切り替え時間(フレーム)


    // 描画レイアウト用の定数
    constexpr int kResultTextY = Statistics::kScreenHeight * 0.1f;  // "RESULT"文字のY座標
    constexpr int kItemStartY = kResultTextY + Statistics::kScreenHeight * 0.2f;
    constexpr int kItemOffsetY = Statistics::kScreenHeight * 0.08f;
    constexpr int kNextSceneTextY = kResultTextY + Statistics::kScreenHeight * 0.6f;

    //constexpr float kLabelTargetDrawX = Statistics::kScreenWidth * 0.3f;
    //constexpr float kValueTargetDrawX = Statistics::kScreenWidth * 0.7f;
    constexpr float kLabelTargetDrawX = Statistics::kScreenWidth * 0.25f;
    constexpr float kValueTargetDrawX = Statistics::kScreenWidth * 0.75f;
    constexpr float kLabelStartDrawX = kLabelTargetDrawX + Statistics::kScreenWidth * 1.5f;
    constexpr float kValueStartDrawX = kValueTargetDrawX + Statistics::kScreenWidth * 1.5f;
}

ResultDisplay::ResultDisplay() :
    _resultFontHandle(-1),
    _animationState(SlideAnimationState::SlidingIn),
    _animationTimer(0),
    _labelDrawX(kLabelStartDrawX),
    _valueDrawX(kValueStartDrawX),
    _resultItems(),
    _nextSceneTextTickFrame(0),
    _isNextSceneTextActive(false),
    _backgroundHandle(-1)
{
    _resultFontHandle = CreateFontToHandle(kFontName.c_str(), kResultFontSize, kFontThickness,
        DX_FONTTYPE_ANTIALIASING_EDGE);
    assert(_resultFontHandle >= 0 && "フォントの作成に失敗");

    _nextSceneFontHandle = CreateFontToHandle(kFontName.c_str(), kNextSceneFontSize, kFontThickness,
        DX_FONTTYPE_ANTIALIASING_EDGE);
    assert(_nextSceneFontHandle >= 0 && "フォントの作成に失敗");
}

ResultDisplay::~ResultDisplay()
{
    if (_resultFontHandle != -1) {
        DeleteFontToHandle(_resultFontHandle);
    }
    if (_nextSceneFontHandle != -1) {
        DeleteFontToHandle(_nextSceneFontHandle);
    }
}

void ResultDisplay::Init()
{
    _backgroundHandle = LoadGraph(L"data/graph/background/BackgroundTitle.png");
    assert(_backgroundHandle >= 0);

    // GameManagerから値を取得
    int enemyDefeatScore = GameManager::GetInstance().GetEnemyDefeatScore();
    int timeBonus = GameManager::GetInstance().GetTimeBonusScore();
    float clearTime = GameManager::GetInstance().GetClearTime();
    int totalScore = GameManager::GetInstance().GetTotalScore();

    // 表示項目を生成してリストに追加
    _resultItems.push_back(std::make_unique<ResultItemDrawer>(
        ResultItemDrawer::ValueType::Number, kScoreText, static_cast<float>(enemyDefeatScore)));
    _resultItems.push_back(std::make_unique<ResultItemDrawer>(
        ResultItemDrawer::ValueType::Time, kTimeText, clearTime));
    _resultItems.push_back(std::make_unique<ResultItemDrawer>(
        ResultItemDrawer::ValueType::Number, kTimeBonusText, static_cast<float>(timeBonus)));
    _resultItems.push_back(std::make_unique<ResultItemDrawer>(
        ResultItemDrawer::ValueType::Number, kTotalScoreText, static_cast<float>(totalScore)));
}

void ResultDisplay::NormalUpdate()
{
    _animationTimer++;

    switch (_animationState)
    {
    case SlideAnimationState::SlidingIn:
    {
        UpdateSlidingIn();
    }
    break;

    case SlideAnimationState::AnimatingItems:
    {
        bool allFinished = true;
        for (const auto& item : _resultItems) {
            item->Update(_animationTimer);
            if (!item->IsAnimationFinished()) {
                allFinished = false;
            }
        }
        if (allFinished) {
            _animationState = SlideAnimationState::Finished;
            _isNextSceneTextActive = true;
        }
    }
    break;

    case SlideAnimationState::Finished:

        _nextSceneTextTickFrame++;

        // 文字描画状態切り替え
        // 表示されている　　かつ　出現時間を超えている
        // または
        // 表示されていない　かつ　消滅時間を超えているなら
        if ((_isNextSceneTextActive &&
            _nextSceneTextTickFrame >= kNextSceneFontThicknessIn) ||
            (!_isNextSceneTextActive &&
                _nextSceneTextTickFrame >= kNextSceneFontThicknessOut)) {

            // 時間をリセットし描画状態を反転させる
            _nextSceneTextTickFrame = 0;
            _isNextSceneTextActive = !_isNextSceneTextActive;
        }
        break;
    }
}

void ResultDisplay::FadeoutUpdate()
{
    // フェードアウト時は案内のアニメーションのみ行う
    _nextSceneTextTickFrame++;

    // 文字描画状態切り替え
    // 点滅切り替え時間を超えているなら
    if (_nextSceneTextTickFrame >= kNextSceneFontThicknessChange) {
        // 時間をリセットし描画状態を反転させる
        _nextSceneTextTickFrame = 0;
        _isNextSceneTextActive = !_isNextSceneTextActive;
    }
}

void ResultDisplay::Draw()
{
    // 背景描画
    DrawGraph(0, 0, _backgroundHandle, true);
    // 背景を暗くする
    constexpr float rate = 0.3f;
    SetDrawBlendMode(DX_BLENDMODE_MULA, static_cast<int>(255 * rate));
    DrawBox(0, 0, Statistics::kScreenWidth, Statistics::kScreenHeight, 0x000000, true);
    // BlendModeを使った後はNOBLENDにしておくことを忘れず
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // "RESULT" の描画
    int resultTextWidth = GetDrawStringWidthToHandle(
        kResultText.c_str(), 
        static_cast<int>(kResultText.length()),
        _resultFontHandle);
    int resultDrawX = static_cast<int>((Statistics::kScreenWidth - resultTextWidth) * 0.5f);
    DrawStringToHandle(
        resultDrawX, kResultTextY, 
        kResultText.c_str(), kTextColor,
        _resultFontHandle);

    // 各項目の描画
    for (int i = 0; i < _resultItems.size(); ++i) {
        float drawY = static_cast<float>(kItemStartY + (kItemOffsetY * i));
        _resultItems[i]->Draw(_labelDrawX, _valueDrawX, drawY);
    }

    // 消滅中は描画を行わない
    if (!_isNextSceneTextActive) return;

    // 最後の入力に応じて文字を変える
    std::wstring drawString = kPadNextSceneText;
    if (Input::GetInstance().GetLastInputType() == Input::PeripheralType::keybd) {
        drawString = kKeybdNextSceneText;
    }

    // 同様に描画する
    int nextSceneTextWidth = GetDrawStringWidthToHandle(
        drawString.c_str(),
        static_cast<int>(drawString.length()),
        _nextSceneFontHandle);
    int NextSceneDrawX = static_cast<int>((Statistics::kScreenWidth - nextSceneTextWidth) * 0.5f);
    DrawStringToHandle(
        NextSceneDrawX, kNextSceneTextY,
        drawString.c_str(), kTextColor,
        _nextSceneFontHandle);
}

void ResultDisplay::UpdateSlidingIn()
{
    float progress = std::min<float>(
        static_cast<float>(_animationTimer) / kSlideInDuration,
        1.0f);
    // イージングで滑らかな動きに
    float easedProgress = 1.0f - powf(1.0f - progress, 3.0f);

    float startX = Statistics::kScreenWidth * 1.5f;
    
    _labelDrawX = startX + (kLabelTargetDrawX - startX) * easedProgress;
    _valueDrawX = startX + (kValueTargetDrawX - startX) * easedProgress;

    if (progress >= 1.0f) {
        _animationState = SlideAnimationState::AnimatingItems;
        _animationTimer = 0;
    }
}
