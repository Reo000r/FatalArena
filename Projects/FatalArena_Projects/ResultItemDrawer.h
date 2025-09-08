#pragma once
#include <string>

class ResultItemDrawer
{
public:
	// 表示する値の種類をコンストラクタで指定する
	enum class ValueType {
		Number, // 汎用的な整数
		Time    // MM:SS.ss 形式で表示する
	};

	ResultItemDrawer(ValueType type, const std::wstring& label, float value);
	~ResultItemDrawer();

	void Update(int animationTimer);
	void Draw(float labelX, float valueX, float baseY) const;

	bool IsAnimationFinished() const { return _isAnimationFinished; }

private:
	// アニメーション処理
	void UpdateNumberAnimation(int animationTimer);
	void UpdateTimeAnimation(int animationTimer);

	// 描画処理
	void DrawNumber(float valueX, float baseY) const;
	void DrawTime(float valueX, float baseY) const;

	/// <summary>
	/// float型の秒数を MMSSss 形式の整数に変換する
	/// </summary>
	/// <param name="timeInSeconds">変換したい秒数</param>
	/// <returns>MMSSss 形式の整数</returns>
	int ConvertTimeToDisplayInt(float timeInSeconds) const;

private:

	std::wstring _label;    // 表示文字
	ValueType _type;        // 値の種類

	float _finalValue;      // 最終的な値
	float _displayValue;    // アニメーション中の表示用値

	int _fontHandle;
	bool _isAnimationFinished;
};
