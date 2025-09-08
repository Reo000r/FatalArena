#pragma once

/// <summary>
/// ポップアップの基底クラス
/// </summary>
class PopupBase
{
public:
	PopupBase() {};

	/// <summary>
	/// 初期化
	/// </summary>
	virtual void Init() abstract;

	/// <summary>
	/// 内部変数の更新
	/// </summary>
	virtual void Update() abstract;

	/// <summary>
	/// 描画全般
	/// </summary>
	virtual void Draw() abstract;
	
	/// <summary>
	/// 終了処理
	/// </summary>
	virtual void Final() abstract;

	/// <summary>
	/// 処理が全て完了したか
	/// </summary>
	/// <returns></returns>
	virtual bool IsProcessingCompleted() abstract;
};

