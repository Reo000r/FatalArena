#include "SceneGamePlay.h"
#include "SceneResult.h"
#include "SceneController.h"
#include "Camera.h"
#include "Player.h"
#include "Arena.h"
#include "Skydome.h"
#include "Physics.h"
#include "DebugDraw.h"
#include "WaveManager.h"
#include "EnemyManager.h"
#include "ItemManager.h"
#include "PlayerBuffManager.h"
#include "WaveAnnouncer.h"
#include "PopupManager.h"
#include "PopupPlayerReinforcement.h"
#include "EnemyFactory.h"
#include "ItemFactory.h"
#include "BillboardManager.h"
#include "StatusUI.h"
#include "GameManager.h"
#include "SoundManager.h"

#include "Statistics.h"
#include "Input.h"

#include <memory>
#include <DxLib.h>
#include <cassert>

SceneGamePlay::SceneGamePlay() :
	_frame(Statistics::kFadeInterval),
	_isProgress(true),
	_nextScene(std::make_shared<SceneResult>()),
	_nowPhase(Phase::Starting),
	_clearState(ClearState::InProgress),
	_physics(std::make_shared<Physics>()),
	_camera(std::make_shared<Camera>()),
	_player(std::make_shared<Player>()),
	_arena(std::make_shared<Arena>()),
	_skydome(std::make_unique<Skydome>()),
	_waveManager(std::make_shared<WaveManager>()),
	_enemyManager(std::make_shared<EnemyManager>()),
	_itemManager(std::make_shared<ItemManager>()),
	_playerBuffManager(std::make_shared<PlayerBuffManager>()),
	_waveAnnouncer(std::make_shared<WaveAnnouncer>()),
	_popupManager(std::make_shared<PopupManager>()),
	_billboardManager(std::make_shared<BillboardManager>()),
	_statusUI(std::make_unique<StatusUI>()),
	_nowUpdateState(&SceneGamePlay::FadeinUpdate),
	_nowDrawState(&SceneGamePlay::FadeinDraw),
	_nowGameDrawState(&SceneGamePlay::StartingGameDraw)
{
}

SceneGamePlay::~SceneGamePlay()
{
	// 敵のモデルを解放する
	EnemyFactory::ReleaseResources();
	// アイテムのモデルを解放する
	ItemFactory::ReleaseResources();
}

void SceneGamePlay::Init()
{
	// 敵のモデルを読み込む
	EnemyFactory::LoadResources();
	// アイテムのモデルを読み込む
	ItemFactory::LoadResources();

	// 初期化処理
	_camera->Init(_player);
	_player->Init(_camera, _physics, _playerBuffManager);
	_skydome->Init(_camera);
	_arena->Init(_physics);

	_enemyManager->Init(_player, _physics);
	_playerBuffManager->Init(_player);
	_itemManager->Init(_physics, _playerBuffManager);
	_waveManager->Init(_enemyManager, _itemManager, _waveAnnouncer);
	_billboardManager->Init();
	_statusUI->Init(_player, _waveManager, _enemyManager);

	GameManager::GetInstance().Init(_player, _waveManager);

	SoundManager::GetInstance().PlaySoundType(BGMType::GamePlay, true, false);
}

void SceneGamePlay::Update()
{
	// ポップアップを操作する可能性があるなら
	if (_waveManager->CanReinforcement()) {

		// ゲームが進行中なら
		if (_isProgress) {
			// ゲームの進行を一時的に止める
			_isProgress = false;
			// ポップアップを開始する
			std::shared_ptr<PopupPlayerReinforcement> popup =
				std::make_shared<PopupPlayerReinforcement>();
			_popupManager->StartPopup(popup);
		}
		// ゲームが進行中でないなら
		else {
			// ポップアップを行っていなければ
			if (!_popupManager->IsPopup()) {
				// ゲームを再び進行させる
				_isProgress = true;
				_waveManager->StartCleanup();
			}
		}

	}
	

	// ゲームが進行中なら
	if (_isProgress) {

		(this->*_nowUpdateState)();

	}
	// でなければ(ゲームが停止していれば)
	else {
		_popupManager->Update();
	}
}

void SceneGamePlay::Draw()
{
	(this->*_nowDrawState)();

	// ゲームが停止していたら
	if (!_isProgress) {

		// 背景を暗くしポップアップを表示する
		SetDrawBlendMode(DX_BLENDMODE_MULA, static_cast<int>(0.5f));
		DrawBox(0, 0, Statistics::kScreenWidth, Statistics::kScreenHeight, 0x000000, true);
		// BlendModeを使った後はNOBLENDにしておくことを忘れず
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		_popupManager->Draw();
	}
}

void SceneGamePlay::FadeinUpdate()
{
	_frame--;

	// フェードが終わったらシーン遷移
	if (_frame <= 0) {
		_nowUpdateState = &SceneGamePlay::NormalUpdate;
		_nowDrawState = &SceneGamePlay::NormalDraw;
		return;
	}

	// 開始前の更新
	StartingUpdate();
}

void SceneGamePlay::NormalUpdate()
{
	switch (_nowPhase) {
	case Phase::Starting:
		StartingUpdate();
		if (_camera->IsCompleteStartAnimation()) {	// 開始演出が終わったら
			_nowPhase = Phase::InProgress;	// ゲーム中
			_nowGameDrawState = &SceneGamePlay::NormalGameDraw;
			_camera->AdvanceState();		// カメラのステートを進める
			_waveManager->StartAnnounce();
		}
		break;
	case Phase::InProgress:
		InProgressUpdate();
		break;
	case Phase::Ending:
		// 終了後の更新を行う
		EndingUpdate();
		break;
	default:
		assert(false);
	}

	// アニメーション中なら
	if (_camera->IsAnimationInProgress()) {
		// フェードアウトが可能な場合は
		if (_camera->CanFadeout()) {
			// フェードアウト準備
			if (_clearState == ClearState::Complete) {
				_nowUpdateState = &SceneGamePlay::FadeoutUpdate;
				_nowDrawState = &SceneGamePlay::FadeoutDraw;
				_frame = 0;
			}
			else if (_clearState == ClearState::Failed) {
				// (クリア時と同じシーンに飛ばしている)
				_nowUpdateState = &SceneGamePlay::FadeoutUpdate;
				_nowDrawState = &SceneGamePlay::FadeoutDraw;
				_frame = 0;
			}
			else {
				assert(false && "不明なステート");
			}
		}
	}
	// でなければ
	else {
		// クリア条件を満たしたら
		if (_waveManager->IsClear()
#ifdef _DEBUG
			|| Input::GetInstance().IsTrigger("Debug::NextScene1")	// 決定を押したら
#endif // _DEBUG
			) {
			_clearState = ClearState::Complete;
			_nowPhase = Phase::Ending;	// 終了中
			_nowGameDrawState = &SceneGamePlay::EndingGameDraw;
			_camera->AdvanceState();	// カメラのステートを進める
		}
		// 失敗条件を満たしたら
		else if (!_player->IsAlive()
#ifdef _DEBUG
			|| Input::GetInstance().IsTrigger("Debug::NextScene2")
#endif // _DEBUG
			) {
			_clearState = ClearState::Failed;
			_nowPhase = Phase::Ending;	// 終了中
			_nowGameDrawState = &SceneGamePlay::EndingGameDraw;
			_camera->AdvanceState();	// カメラのステートを進める
		}
	}
	
}

void SceneGamePlay::FadeoutUpdate()
{
	_frame++;

	// 遷移条件を満たしたら
	if (_frame >= Statistics::kFadeInterval &&
		true) {
		SceneController::GetInstance().ChangeScene(_nextScene);
		return;  // 自分が死んでいるのでもし
		// 余計な処理が入っているとまずいのでreturn;
	}

	// 終了後の更新を行う
	EndingUpdate();
}

void SceneGamePlay::FadeinDraw()
{
	// ゲーム内容を描画する
	(this->*_nowGameDrawState)();

#ifdef _DEBUG
	//DrawFormatString(0, 0, 0xffffff, L"Scene GamePlay");
#endif

	// フェードイン/アウトの処理
	// フェード割合の計算(0.0-1.0)
	float rate = static_cast<float>(_frame) / static_cast<float>(Statistics::kFadeInterval);
	SetDrawBlendMode(DX_BLENDMODE_MULA, static_cast<int>(255 * rate));
	DrawBox(0, 0, Statistics::kScreenWidth, Statistics::kScreenHeight, 0x000000, true);
	// BlendModeを使った後はNOBLENDにしておくことを忘れず
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void SceneGamePlay::FadeoutDraw()
{
	// ゲーム内容を描画する
	(this->*_nowGameDrawState)();

#ifdef _DEBUG
	//DrawFormatString(0, 0, 0xffffff, L"Scene GamePlay");
#endif

	// フェードイン/アウトの処理
	// フェード割合の計算(0.0-1.0)
	float rate = static_cast<float>(_frame) / static_cast<float>(Statistics::kFadeInterval);
	SetDrawBlendMode(DX_BLENDMODE_MULA, static_cast<int>(255 * rate));
	DrawBox(0, 0, Statistics::kScreenWidth, Statistics::kScreenHeight, 0x000000, true);
	// BlendModeを使った後はNOBLENDにしておくことを忘れず
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void SceneGamePlay::NormalDraw()
{
	// ゲーム内容を描画する
	(this->*_nowGameDrawState)();

#ifdef _DEBUG
	//DrawFormatString(0, 0, 0xffffff, L"Scene GamePlay");
#endif
}

void SceneGamePlay::StartingUpdate()
{
	// 開始時の更新を行う
	// カメラ演出、ビルボードアニメーション
	_camera->Update();
	_skydome->Update();
	_billboardManager->Update();
}

void SceneGamePlay::InProgressUpdate()
{
	// ゲーム中の更新を行う

	// クリアタイムの加算
	GameManager::GetInstance().UpdateClearTime();

	// 更新
	_camera->Update();
	_skydome->Update();

	_player->Update();

	_statusUI->Update();
	_enemyManager->Update();
	_itemManager->Update();
	_playerBuffManager->Update();
	_waveAnnouncer->Update();
	_waveManager->Update();

	// 物理演算更新
	_physics->Update();
}

void SceneGamePlay::EndingUpdate()
{
	// 終了後の更新を行う
	// カメラ演出、ビルボードアニメーション
	_camera->Update();
	_skydome->Update();
	_billboardManager->Update();
}

void SceneGamePlay::StartingGameDraw()
{
	_skydome->Draw();
	_arena->Draw();
	_billboardManager->Draw();

	_camera->Draw();
	_player->Draw();

	_enemyManager->Draw();
	_itemManager->Draw();
}

void SceneGamePlay::EndingGameDraw()
{
	_skydome->Draw();
	_arena->Draw();
	_billboardManager->Draw();

	_camera->Draw();
	_player->Draw();

	_enemyManager->Draw();
	_itemManager->Draw();
}

void SceneGamePlay::NormalGameDraw()
{
	_skydome->Draw();
	_arena->Draw();

	_camera->Draw();
	_player->Draw();

	_enemyManager->Draw();
	_itemManager->Draw();
	_statusUI->Draw();
	_playerBuffManager->Draw();
	_waveAnnouncer->Draw();
}

