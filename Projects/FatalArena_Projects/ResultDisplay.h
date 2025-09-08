#pragma once
#include <vector>
#include <memory>

class ResultItemDrawer;

class ResultDisplay
{
private:
	/// <summary>
	/// アニメーション状態
	/// </summary>
	enum class SlideAnimationState {
		SlidingIn,  // スライドイン中
		AnimatingItems,// アニメーション中
		Finished    // アニメーション完了
	};

public:
	ResultDisplay();
	~ResultDisplay();

	/// <summary>
	/// 初期化
	/// </summary>
	void Init();

	/// <summary>
	/// 通常の更新
	/// </summary>
	void NormalUpdate();
	/// <summary>
	/// フェードアウト時の更新
	/// </summary>
	void FadeoutUpdate();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// アニメーション処理が終了しているか返す
	/// </summary>
	/// <returns></returns>
	bool IsAnimationFinished() { return _animationState == SlideAnimationState::Finished; }

private:

	/// <summary>
	/// 更新処理の補助関数
	/// </summary>
	void UpdateSlidingIn();

	int _resultFontHandle;
	int _nextSceneFontHandle;

	SlideAnimationState _animationState;	// 現在のアニメーション状態
	int _animationTimer;            // アニメーション進行度タイマー
	
	float _labelDrawX;
	float _valueDrawX;

	std::vector<std::unique_ptr<ResultItemDrawer>> _resultItems;

	int _nextSceneTextTickFrame;
	bool _isNextSceneTextActive;

	int _backgroundHandle;
};
