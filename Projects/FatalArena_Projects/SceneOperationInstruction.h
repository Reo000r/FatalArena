#pragma once
#include "SceneBase.h"
#include <memory>
#include <string>
#include <vector>

/// <summary>
/// 操作説明を行うシーン
/// </summary>
class SceneOperationInstruction final : public SceneBase
{
public:
	SceneOperationInstruction();
	~SceneOperationInstruction();

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

private:

	/// <summary>
	/// フェードイン時の更新処理
	/// </summary>
	void FadeinUpdate();
	/// <summary>
	/// 通常時の更新処理
	/// </summary>
	void NormalUpdate();
	/// <summary>
	/// フェードアウト時の更新処理
	/// </summary>
	void FadeoutUpdate();

	/// <summary>
	/// フェード時の描画
	/// </summary>
	void FadeDraw();
	/// <summary>
	/// 通常時の描画
	/// </summary>
	void NormalDraw();


	int _frame;

	// _updateや_drawが変数であることを分かりやすくしている
	using UpdateFunc_t = void (SceneOperationInstruction::*)();
	using DrawFunc_t = void (SceneOperationInstruction::*)();
	UpdateFunc_t _nowUpdateState = nullptr;
	DrawFunc_t _nowDrawState = nullptr;

	enum class NextSceneName {
		GamePlay,		// ゲームシーン
		Title,			// タイトル
	};

	NextSceneName _nextSceneName;
	std::shared_ptr<SceneBase> _nextScene;

private:

	/// <summary>
	/// 操作説明を描画
	/// </summary>
	void DrawOperationInstructionString();


	// 文字ハンドル
	int _headingFontHandle;			// 見出し文字ハンドル
	int _subheadingFontHandle;		// 小見出し文字ハンドル
	int _descriptionFontHandle;		// 詳細文字ハンドル
	int _nextSceneFontHandle;		// シーン遷移案内文字ハンドル

	std::vector<std::wstring> _subheadingString;	// 小見出し文字
	std::vector<std::wstring> _descriptionString;	// 詳細文字

	// 演出用変数
	int _nextSceneTextTickFrame;	// 文字点滅時間を管理
	bool _isNextSceneTextActive;	// 文字描画を行うか

	int _backgroundHandle;
	int _controllerGraphHandle;
};

