#pragma once
#include <memory>

class PopupBase;

/// <summary>
/// 登録されたポップアップを管理する
/// Initを行ってからセット
/// </summary>
class PopupManager
{
public:
	PopupManager();
	~PopupManager();

	/// <summary>
	/// ポップアップを始める
	/// </summary>
	/// <param name="popupBase"></param>
	void StartPopup(std::shared_ptr<PopupBase> popupBase);

	/// <summary>
	/// 内部変数の更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画全般
	/// </summary>
	void Draw();

	/// <summary>
	/// ポップアップを行っているか
	/// </summary>
	/// <returns></returns>
	bool IsPopup() const { return _isPopup; }

private:

	/// <summary>
	/// ポップアップが入った時の更新処理
	/// </summary>
	void PopinUpdate();
	/// <summary>
	/// 通常時の更新処理
	/// </summary>
	void NormalUpdate();
	/// <summary>
	/// ポップアップから出る時の更新処理
	/// </summary>
	void PopoutUpdate();

	/// <summary>
	/// 出入り時の描画
	/// </summary>
	void FadeDraw();
	/// <summary>
	/// 通常時の描画
	/// </summary>
	void NormalDraw();

	int _frame;
	bool _isPopup;

	// _updateや_drawが変数であることを分かりやすくしている
	using UpdateFunc_t = void (PopupManager::*)();
	using DrawFunc_t = void (PopupManager::*)();
	UpdateFunc_t _nowUpdateState = nullptr;
	DrawFunc_t _nowDrawState = nullptr;

private:

	// ポップアップ内容
	std::shared_ptr<PopupBase> _popup;

};

