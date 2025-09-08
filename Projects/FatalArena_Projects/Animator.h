#pragma once

#include <string>
#include <forward_list>

class Animator
{
public:
	/// <summary>
	/// アニメーションデータ構造体
	/// </summary>
	struct AnimData
	{
		int animIndex = -1;			// アニメーション番号(元データにおける)
		int attachNo = -1;			// アタッチ番号
		std::wstring animName = L"";	// アニメーションの名前
		float animSpeed = 1.0f;		// アニメーションの再生速度
		float frame = 0.0f;			// アニメーションの再生時間
		float totalFrame = 0.0f;	// アニメーションの総再生時間
		bool  isLoop = false;		// ループするか
		bool  isEnd = false;		// ループしない場合終了しているか

		float inputAcceptanceStartFrame = 0.0f; // 入力受付開始フレーム
		float inputAcceptanceEndFrame = 0.0f;   // 入力受付終了フレーム
	};

	Animator();
	~Animator();

	void Init(int model);
	void Update();

	/// <summary>
	/// 最初に使用するアニメーションを設定
	/// </summary>
	/// <param name="animName"></param>
	void SetStartAnim(const std::wstring animName);
	/// <summary>
	/// ゲーム中で使用するアニメーションデータ
	/// </summary>
	void SetAnimData(const std::wstring animName, const float animSpeed, const bool isLoop, 
		float inputAcceptanceStartRatio = 0.0f, 
		float inputAcceptanceEndRatio = 1.0f);
	/// <summary>
	/// アニメーション名を指定しアタッチ
	/// (ブレンドの進行状況が止まるため初期化する目的で使用)
	/// </summary>
	/// <param name="animName"></param>
	/// <param name="isLoop"></param>
	void AttachAnim(const std::wstring animName, const bool isLoop);

	/// <summary>
	/// 指定されたアニメーションの更新
	/// </summary>
	/// <param name="data"></param>
	void UpdateAnim(AnimData& data);

	/// <summary>
	/// ブレンド比率の更新
	/// </summary>
	void UpdateAnimBlendRate();

	/// <summary>
	/// 
	/// </summary>
	/// <param name="animName"></param>
	/// <param name="isLoop"></param>
	void ChangeAnim(const std::wstring animName, bool isLoop);

	/// <summary>
	/// アニメーションデータを名前で検索し参照を返す
	/// 見つからなかった場合は最初に見たアニメーションデータを返す
	/// debugならassertを投げる
	/// </summary>
	/// <param name="animName"></param>
	/// <returns></returns>
	AnimData& FindAnimData(const std::wstring animName);

	int GetModelHandle() const{ return _model; }

	std::wstring GetCurrentAnimName() const{ return _currentAnimName; }
	
	float GetCurrentAnimFrame();

	/// <summary>
	/// 指定のアニメーションが終了しているか
	/// </summary>
	/// <param name="animName"></param>
	/// <returns></returns>
	bool IsEnd(const std::wstring animName) { return FindAnimData(animName).isEnd; }
	/// <summary>
	/// 指定のアニメーションがループするか
	/// </summary>
	/// <param name="animName"></param>
	/// <returns></returns>
	bool IsLoop(const std::wstring animName) { return FindAnimData(animName).isLoop; }

private:
	// モデルハンドル
	int _model;
	
	// アニメーションのforward_list(list)
	// 順番を気にする必要がないかつ
	// メモリの効率化のためforward_list
	std::forward_list<AnimData> _animDataList;

	// 現在再生中のアニメーション名
	std::wstring _currentAnimName;
	// 前に再生されていてブレンドアウトしていくアニメーション名
	std::wstring _prevAnimName;

	// アニメーションのブレンド比率
	// _currentAnimName のウェイトとして使用する
	// 0.0->1.0
	float _blendRate;
};

