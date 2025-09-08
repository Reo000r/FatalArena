#pragma once
#include "Vector3.h"

#include <map>
#include <string>
#include <vector>

/// <summary>
/// 入力系をコントロールするクラス
/// シングルトン化する
/// </summary>
class Input final {
public:
	/// <summary>
	/// シングルトンオブジェクトを返す
	/// </summary>
	/// <returns>シングルトンオブジェクト</returns>
	static Input& GetInstance();

	/// <summary>
	/// 入力情報の更新
	/// </summary>
	void Update();

	// 周辺機器種別
	enum class PeripheralType {
		keybd,	// キーボード
		pad1	// パッド１
	};

	/// <summary>
	/// ゲーム起動時にデフォルトにする入力種別を設定
	/// </summary>
	/// <param name="type"></param>
	void SetInputType(PeripheralType type);

	/// <summary>
	/// 最後に入力された機器を返す
	/// </summary>
	/// <returns></returns>
	PeripheralType GetLastInputType() const;

	/// <summary>
	/// 押されているかどうかの取得
	/// </summary>
	/// <param name="button">判定を行うボタン</param>
	/// <returns>押されていればtrue、でなければfalse</returns>
	bool IsPress(const char* key) const;

	/// <summary>
	/// 押された瞬間かどうかの取得
	/// </summary>
	/// <param name="button">判定を行うボタン</param>
	/// <returns>押されていればtrue、でなければfalse</returns>
	bool IsTrigger(const char* key) const;

	/// <summary>
	/// 右スティックの入力情報をVector3型で返す
	/// x、zに値を入れる
	/// </summary>
	/// <returns>右スティックの入力情報</returns>
	Vector3 GetPadRightSitck() const;
	/// <summary>
	/// 右スティックの入力情報をVector3型で返す
	/// x、zに値を入れる
	/// 直前にあった入力を返す
	/// </summary>
	/// <returns>右スティックの入力情報</returns>
	Vector3 GetPadRightSitckLast() const;

	/// <summary>
	/// 左スティックの入力情報をVector3型で返す
	/// x、zに値を入れる
	/// </summary>
	/// <returns>左スティックの入力情報</returns>
	Vector3 GetPadLeftSitck() const;
	/// <summary>
	/// 左スティックの入力情報をVector3型で返す
	/// x、zに値を入れる
	/// 直前にあった入力を返す
	/// </summary>
	/// <returns>左スティックの入力情報</returns>
	Vector3 GetPadLeftSitckLast() const;

	/// <summary>
	/// 右クリックが行われているか取得
	/// </summary>
	/// <returns>押されていればtrue、でなければfalse</returns>
	bool IsPressMouseRightClick() const;
	/// <summary>
	/// 左クリックが行われているか取得
	/// </summary>
	/// <returns>押されていればtrue、でなければfalse</returns>
	bool IsPressMouseLeftClick() const;
	/// <summary>
	/// ホイールクリックが行われているか取得
	/// </summary>
	/// <returns>押されていればtrue、でなければfalse</returns>
	bool IsPressMouseMiddleClick() const;

	/// <summary>
	/// 右クリックが行われた瞬間であるか取得
	/// </summary>
	/// <returns>押されていればtrue、でなければfalse</returns>
	bool IsTriggerMouseRightClick() const;
	/// <summary>
	/// 左クリックが行われた瞬間であるか取得
	/// </summary>
	/// <returns>押されていればtrue、でなければfalse</returns>
	bool IsTriggerMouseLeftClick() const;
	/// <summary>
	/// ホイールクリックが行われた瞬間であるか取得
	/// </summary>
	/// <returns>押されていればtrue、でなければfalse</returns>
	bool IsTriggerMouseMiddleClick() const;

	/// <summary>
	/// マウスの位置情報をVector3型で返す
	/// x、zに値を入れる
	/// </summary>
	/// <returns></returns>
	Vector3 GetMousePosition() const;
	/// <summary>
	/// マウスの位置情報をVector3型で返す
	/// x、zに値を入れる
	/// 直前にあった入力を返す
	/// </summary>
	/// <returns></returns>
	Vector3 GetMousePositionLast() const;

private:
	Input();
	Input(const Input&) = delete;
	void operator=(const Input&) = delete;

	///入力情報定義用
	struct InputState {
		PeripheralType type;	// 周辺機器種別
		int id;					// 実際の入力を表す定数
	};

	using InputTable_t = std::map<std::string, std::vector<InputState>>;
	InputTable_t _inputTable;		// 名前と実際の入力の対応表
	InputTable_t _tempInputTable;	// 名前と実際の入力の対応表(一時的なコピー)

	/// <summary>
	/// 入力を初期値に設定する
	/// </summary>
	void SetDefault();

	///// <summary>
	///// キーコンフィグ用(行った変更を確定する)
	///// </summary>
	//void CommitEdittedInputTable();

	///// <summary>
	///// キーコンフィグ用(行った変更をなかったことにする)
	///// </summary>
	//void RollbackEdittedInputTable();

	// 押されたかどうかを記録していくもの
	using InputRecord_t = std::map<std::string, bool>;
	InputRecord_t _current;		// 現在押してるかどうか
	InputRecord_t _last;		// 直前に押されてたかどうか

	std::vector<std::string> _orderForDisplay;	// 表示用(mapの順序はコントロールできないため)

	// 最後に入力された機器
	PeripheralType _lastInputType;

	// 入力保存
	int _currentRawPadState;			// 今の生パッドステート
	int _lastRawPadState;				// 直前の生パッドステート
	char _currentRawKeybdState[256];	// 今のキーボード状態
	char _lastRawKeybdState[256];		// 直前のキーボード状態

	// 左右スティック入力情報
	Vector3 _currentRightStickInput;
	Vector3 _lastRightStickInput;
	Vector3 _currentLeftStickInput;
	Vector3 _lastLeftStickInput;

	// マウス入力情報
	int _currentRawMouseState;
	int _lastRawMouseState;
	Vector3 _currentMousePosition;
	Vector3 _lastMousePosition;

	/// <summary>
	/// 現在のキーボード状態を調べて最も
	/// 若いキーコードを返す
	/// ひとつも押されてなければ-1を返す
	/// </summary>
	/// <returns></returns>
	int GetKeyboradState()const;

	/// <summary>
	/// 現在のPAD状態を調べて最も
	/// 若いキー入力を返す
	/// ひとつも押されてなければ-1を返す
	/// </summary>
	int GetPadState(int padno)const;

	/// <summary>
	/// 現在の入力テーブルをセーブする
	/// </summary>
	void SaveInputTable();

	/// <summary>
	/// 入力テーブルをロードして反映する
	/// </summary>
	void LoadInputTable();
};