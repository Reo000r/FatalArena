#include "ResultItemDrawer.h"
#include "Statistics.h"
#include <DxLib.h>
#include <cassert>

namespace {
    // 定数
    const unsigned int kTextColor = GetColor(255, 255, 255);
    const std::wstring kFontName = Statistics::kDefaultFontName;
    constexpr int kFontSize = 64;
    constexpr int kFontThickness = 3;
    constexpr int kScoreDigitCount = 7;      // 表示桁数
    constexpr int kTimeDigitCount = 6;       // 表示桁数
    const std::wstring kScoreFormat = L"%07d";      // 0埋め部分
    constexpr int kDigitFinalizeDuration = 20;
}

ResultItemDrawer::ResultItemDrawer(ValueType type, const std::wstring& label, float value) :
    _type(type), 
    _label(label),
    _finalValue(value),
    _displayValue(0.0f),
    _fontHandle(-1),
    _isAnimationFinished(false)
{
    _fontHandle = CreateFontToHandle(kFontName.c_str(), kFontSize, kFontThickness,
        DX_FONTTYPE_ANTIALIASING_EDGE);
    assert(_fontHandle >= 0 && "フォントの作成に失敗");
}

ResultItemDrawer::~ResultItemDrawer()
{
    if (_fontHandle != -1) {
        DeleteFontToHandle(_fontHandle);
    }
}

void ResultItemDrawer::Update(int animationTimer)
{
    // アニメーションが終了していたならreturn
    if (_isAnimationFinished) return;

    // 型に応じてアニメーション処理を分岐
    switch (_type) {
    case ValueType::Number:
        UpdateNumberAnimation(animationTimer);
        break;
    case ValueType::Time:
        UpdateTimeAnimation(animationTimer);
        break;
    }
}

void ResultItemDrawer::Draw(float labelX, float valueX, float baseY) const
{
    // ラベルを描画
    DrawStringToHandle(static_cast<int>(labelX), static_cast<int>(baseY), _label.c_str(),
        kTextColor, _fontHandle);

    // 型に応じて描画処理を分岐
    switch (_type) {
    case ValueType::Number:
        DrawNumber(valueX, baseY);
        break;
    case ValueType::Time:
        DrawTime(valueX, baseY);
        break;
    }
}

void ResultItemDrawer::UpdateNumberAnimation(int animationTimer)
{
    // アニメーションで確定させる桁数を計算
    int finalizeDigitCount = animationTimer / kDigitFinalizeDuration;
    
    // アニメーションが完了した場合
    if ((finalizeDigitCount >= kScoreDigitCount) ||
        _isAnimationFinished) {
        finalizeDigitCount = kScoreDigitCount;
        _isAnimationFinished = true;
        _displayValue = _finalValue;
    }

    // アニメーション中の処理
    int tempValue = 0;
    int finalValueCopy = (int)_finalValue;
    int powerOf10 = 1;

    for (int i = 0; i < kScoreDigitCount; ++i) {
        int digit = 0;
        // 確定済みの桁（下位の桁）は本来の数字
        if (i < finalizeDigitCount) {
            digit = finalValueCopy % 10;
        }
        // 確定していない桁（上位の桁）はランダムな数字
        else {
            digit = GetRand(9); // 0-9
        }
        // 計算結果を加算
        tempValue += digit * powerOf10;
        // 次の桁の前処理
        powerOf10 *= 10;
        finalValueCopy /= 10;
    }

    // 表示用の値を更新
    _displayValue = (float)tempValue;
}

void ResultItemDrawer::UpdateTimeAnimation(int animationTimer)
{
    // アニメーションで確定させる桁数を計算 (MMSSss の6桁)
    int finalizedDigitCount = animationTimer / kDigitFinalizeDuration;
    if (finalizedDigitCount >= kTimeDigitCount) {
        finalizedDigitCount = kTimeDigitCount;
        _isAnimationFinished = true;
    }

    // 最終的なクリアタイムをMMSSss形式の整数に変換
    const int finalTimeInt = ConvertTimeToDisplayInt(_finalValue);

    // アニメーションが完了した場合
    if (_isAnimationFinished) {
        _displayValue = (float)finalTimeInt;
    }
    // アニメーション中の場合
    else {
        int tempTimeInt = 0;
        int powerOf10 = 1;
        int finalTimeCopy = finalTimeInt;


        // 下の桁から順番に処理
        for (int i = 0; i < kTimeDigitCount; ++i) {
            int digit = 0;
            // 確定済みの桁（下位の桁）は本来の数字
            if (i < finalizedDigitCount) {
                digit = finalTimeCopy % 10;
            }
            // 確定していない桁（上位の桁）はランダムな数字
            else {
                digit = GetRand(9); // 0-9
            }

            // 計算した桁の値を tempTimeInt に加算
            tempTimeInt += digit * powerOf10;
            // 次の桁の準備
            powerOf10 *= 10;
            finalTimeCopy /= 10;
        }

        _displayValue = (float)tempTimeInt;
    }
}

void ResultItemDrawer::DrawNumber(float valueX, float baseY) const
{
    // 表示用の数値を、5桁・ゼロ埋めのワイド文字列に変換する
    wchar_t buffer[16]; // "00000" と終端文字が入るのに十分なサイズを確保
    swprintf_s(buffer, kScoreFormat.c_str(), static_cast<int>(_displayValue));

    // 生成した文字列の描画幅（ピクセル数）を取得する
    int numWidth = GetDrawStringWidthToHandle(buffer, (int)wcslen(buffer), _fontHandle);

    // 右揃えで描画するためのX座標を計算する
    // (基準のX座標 - 文字列の幅)
    int drawX = static_cast<int>(valueX) - numWidth;

    // 計算した座標に文字列を描画する
    DrawStringToHandle(drawX, static_cast<int>(baseY), buffer, kTextColor, _fontHandle);
}

void ResultItemDrawer::DrawTime(float valueX, float baseY) const
{
    // 表示用の秒数を、分・秒・ミリ秒に分解する
    const int displayTimeInt = static_cast<int>(_displayValue);
    int minutes = displayTimeInt / 10000;
    int seconds = (displayTimeInt / 100) % 100;
    int milliseconds = displayTimeInt % 100;

    // 各パーツ（分、秒、ミリ秒）を個別の文字列に変換する
    wchar_t buffer[4]; // "00:" など最大4文字が入るサイズ
    swprintf_s(buffer, L"%02d:", minutes);
    std::wstring minStr = buffer;
    swprintf_s(buffer, L"%02d.", seconds);
    std::wstring secStr = buffer;
    swprintf_s(buffer, L"%02d", milliseconds);
    std::wstring msStr = buffer;

    // 各パーツの描画幅を取得する
    int minWidth = GetDrawStringWidthToHandle(minStr.c_str(), (int)minStr.length(), _fontHandle);
    int secWidth = GetDrawStringWidthToHandle(secStr.c_str(), (int)secStr.length(), _fontHandle);
    int msWidth = GetDrawStringWidthToHandle(msStr.c_str(), (int)msStr.length(), _fontHandle);

    // 全体の幅を計算し、右揃えにするための描画開始X座標を求める
    int totalWidth = minWidth + secWidth + msWidth;
    int currentX = static_cast<int>(valueX) - totalWidth;

    // 各パーツを左から順番に描画していく
    DrawStringToHandle(currentX, static_cast<int>(baseY), minStr.c_str(), kTextColor,
        _fontHandle);
    currentX += minWidth; // 次の描画位置を更新
    DrawStringToHandle(currentX, static_cast<int>(baseY), secStr.c_str(), kTextColor,
        _fontHandle);
    currentX += secWidth; // 次の描画位置を更新
    DrawStringToHandle(currentX, static_cast<int>(baseY), msStr.c_str(), kTextColor,
        _fontHandle);
}

int ResultItemDrawer::ConvertTimeToDisplayInt(float timeInSeconds) const
{
    // 最終的なクリアタイムを MMSSss 形式の整数に変換
    int minutes = static_cast<int>(_finalValue) / 60;
    int seconds = static_cast<int>(_finalValue) % 60;
    int milliseconds = static_cast<int>((_finalValue - floor(_finalValue)) * 100.0f);
    return minutes * 10000 + seconds * 100 + milliseconds;

}
