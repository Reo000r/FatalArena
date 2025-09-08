#pragma once
#include "SceneBase.h"
#include "Geometry.h"

#include <memory>

class Physics;
class Camera;
class Player;
class Arena;
class Skydome;
class WaveManager;
class EnemyManager;
class ItemManager;
class PlayerBuffManager;
class WaveAnnouncer;
class PopupManager;
class BillboardManager;
class StatusUI;

class SceneGamePlay final : public SceneBase {
public:
	SceneGamePlay();
	~SceneGamePlay();

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

	int _frame;
	bool _isProgress;

	// UpdateとDrawのStateパターン
	// _updateや_drawが変数であることを分かりやすくしている
	using UpdateFunc_t = void(SceneGamePlay::*)();
	using DrawFunc_t = void(SceneGamePlay::*)();
	UpdateFunc_t _nowUpdateState = nullptr;
	DrawFunc_t _nowDrawState = nullptr;			// シーンの演出描画

	/// <summary>
	/// フェードイン時の更新処理
	/// </summary>
	void FadeinUpdate();
	// 通常時の更新処理
	void NormalUpdate();
	// フェードアウト時の更新処理
	void FadeoutUpdate();

	// フェードイン時の描画
	void FadeinDraw();
	// フェードアウト時の描画
	void FadeoutDraw();
	// 通常時の描画
	void NormalDraw();

	// 次のシーン
	std::shared_ptr<SceneBase> _nextScene;

private:

	DrawFunc_t _nowGameDrawState = nullptr;		// ゲーム内容描画
	enum class Phase {
		Starting,
		InProgress,
		Ending,
	};
	// クリアしたかどうか
	enum class ClearState {
		InProgress,
		Complete,
		Failed,
	};

	/// <summary>
	/// フェードイン中/ゲーム中に呼ばれる更新関数
	/// 条件を満たすまでフェーズを変更しない
	/// </summary>
	void StartingUpdate();
	/// <summary>
	/// ゲーム中に呼ばれる更新関数
	/// 条件を満たすまでフェーズを変更しない
	/// </summary>
	void InProgressUpdate();
	/// <summary>
	/// フェードアウト中に呼ばれる更新関数
	/// </summary>
	void EndingUpdate();

	/// <summary>
	/// 開始時のオブジェクトの描画を行う関数
	/// </summary>
	void StartingGameDraw();
	/// <summary>
	/// 終了時のオブジェクトの描画を行う関数
	/// </summary>
	void EndingGameDraw();
	/// <summary>
	/// オブジェクトの描画を行う関数
	/// </summary>
	void NormalGameDraw();

	// ゲームが現在どのフェーズにあるか
	Phase _nowPhase;
	// クリアしたかどうか
	ClearState _clearState;

	std::shared_ptr<Physics> _physics;

	std::shared_ptr<Camera> _camera;
	std::shared_ptr<Player> _player;
	std::shared_ptr<Arena> _arena;
	std::unique_ptr<Skydome> _skydome;
	std::shared_ptr<WaveManager> _waveManager;
	std::shared_ptr<EnemyManager> _enemyManager;
	std::shared_ptr<ItemManager> _itemManager;
	std::shared_ptr<PlayerBuffManager> _playerBuffManager;
	std::shared_ptr<WaveAnnouncer> _waveAnnouncer;
	std::shared_ptr<PopupManager> _popupManager;
	std::shared_ptr<BillboardManager> _billboardManager;
	std::unique_ptr<StatusUI> _statusUI;

};

