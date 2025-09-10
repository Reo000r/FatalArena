#include "WaveAnnouncer.h"
#include "Statistics.h"
#include "StringUtility.h"
#include "SoundManager.h"
#include <DxLib.h>
#include <string>

namespace
{
    // 定数
    constexpr int kDisplayDuration = 90;    // 全体の表示フレーム
    constexpr int kFadeInDuration = 20;     // フェードインにかかるフレーム
    constexpr int kFadeOutDuration = 20;    // フェードアウトにかかるフレーム
    constexpr int kFontSize = Statistics::kScreenCenterWidth * 0.15f;   // フォントサイズ
    const unsigned int kFontColor = 0xffffff;   // 文字色
    //const std::wstring kFontName = L"Baskerville Old Face";   // フォント名
    const std::wstring kFontName = Statistics::kDefaultFontName;   // フォント名
}

WaveAnnouncer::WaveAnnouncer() :
    _isDisplaying(false),
    _displayTimer(0),
    _currentWave(0),
    _maxWave(0),
    _fontHandle(-1)
{
    // フォントの作成
    _fontHandle = CreateFontToHandle(
        kFontName.c_str(), kFontSize, 3, DX_FONTTYPE_ANTIALIASING_EDGE);
}

WaveAnnouncer::~WaveAnnouncer()
{
    // フォントの削除
    if (_fontHandle != -1)
    {
        DeleteFontToHandle(_fontHandle);
    }
}

void WaveAnnouncer::Update()
{
    if (!_isDisplaying)
    {
        return;
    }

    _displayTimer++;

    if (_displayTimer > kDisplayDuration)
    {
        _isDisplaying = false;
    }
}

void WaveAnnouncer::Draw()
{
    if (!_isDisplaying)
    {
        return;
    }

    // フェード処理
    int alpha = 255;
    if (_displayTimer < kFadeInDuration)
    {
        // フェードイン
        alpha = static_cast<int>(255.0f * (_displayTimer / static_cast<float>(kFadeInDuration)));
    }
    else if (_displayTimer > kDisplayDuration - kFadeOutDuration)
    {
        // フェードアウト
        int remainingTime = kDisplayDuration - _displayTimer;
        alpha = static_cast<int>(255.0f * (remainingTime / static_cast<float>(kFadeOutDuration)));
    }

    // 描画ブレンドモードをアルファブレンドに設定
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

    // 描画処理
    int textWidth = GetDrawFormatStringWidthToHandle(
        _fontHandle, L"Wave  %d / %d", _currentWave, _maxWave);
    int drawX = (Statistics::kScreenWidth - textWidth) / 2;
    int drawY = Statistics::kScreenHeight / 3;

    DrawFormatStringToHandle(
        drawX, drawY, 
        kFontColor, _fontHandle, 
        L"Wave  %d / %d", _currentWave, _maxWave);

    // 描画ブレンドモードを元に戻す
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void WaveAnnouncer::Start(int currentWave, int maxWave)
{
    _currentWave = currentWave;
    _maxWave = maxWave;
    _isDisplaying = true;
    _displayTimer = 0;

    SoundManager::GetInstance().PlaySoundType(SEType::WaveStart);
}

bool WaveAnnouncer::IsFinished() const
{
    return !_isDisplaying;
}
