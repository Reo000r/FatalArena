#include "SceneTitle.h"
#include "SceneGamePlay.h"  // 遷移先のシーン
#include "SceneOperationInstruction.h"  // 遷移先のシーン
#include "SceneController.h"
#include "Physics.h"
#include "Skydome.h"
#include "Arena.h"
#include "BillboardManager.h"
#include "SoundManager.h"

#include "Input.h"
#include "Statistics.h"
#include "Calculation.h"

#include <DxLib.h>
#include <cassert>

namespace {
	const unsigned int kTextColor = GetColor(255, 255, 255);
	const std::wstring kFontName = Statistics::kDefaultFontName;
	//constexpr int kTitleFontSize = 128;				// タイトル文字サイズ
	constexpr int kNextSceneFontSize = 96;
	constexpr int kFontThickness = 3;
	//const std::wstring kTitleText = L"Fatal Arena";
	const std::wstring kPadNextSceneText = L"       Press A to Start\nPress B to Instruction";
	const std::wstring kKeybdNextSceneText = L"      Press Enter to Start\nPress Esc to Instruction";
	//const std::wstring kPadNextSceneText = L"Aボタンでゲームスタート\n    Bボタンで操作説明";
	//const std::wstring kKeybdNextSceneText = L"エンターキーでゲームスタート\n    エスケープキーで操作説明";

	// アニメーション用の定数
	constexpr int kNextSceneFontThicknessIn		= 40;	// 文字の出現期間(フレーム)
	constexpr int kNextSceneFontThicknessOut	= 20;	// 文字の消滅期間(フレーム)
	constexpr int kNextSceneFontThicknessChange	= 4;	// シーン切り替え中の文字の点滅切り替え時間(フレーム)

	// 描画レイアウト用の定数
	constexpr int kTitleTextY = Statistics::kScreenHeight * 0.1f;						// タイトル文字のY座標
	constexpr int kNextSceneTextY = kTitleTextY + Statistics::kScreenHeight * 0.65f;

	const Position3 kCameraPos = Position3(0.0f, 600.0f, 0.0f);
	const Vector3 kTargetPos = Vector3(0.0f, 1000.0f, 1000.0f);
	constexpr float kNear = 10.0f;
	constexpr float kFar = 10000.0f;
	const float kViewAngle = Calc::ToRadian(60.0f);

	const float kCameraTargetRadius = kTargetPos.z;				// カメラの注視点の回転半径
	const float kCameraRotationSpeed = Calc::ToRadian(0.05f);	// カメラの回転速度
}

SceneTitle::SceneTitle() :
	_frame(Statistics::kFadeInterval),
	_nowUpdateState(&SceneTitle::FadeinUpdate),
	_nowDrawState(&SceneTitle::FadeDraw),
	_nextSceneName(NextSceneName::GamePlay),
	_nextScene(nullptr),
	_titleFontHandle(-1),
	_nextSceneFontHandle(-1),
	_nextSceneTextTickFrame(0),
	_isNextSceneTextActive(true),
	_titleImageHandle(-1),
	_cameraAngleY(0.0f),
	_targetPos(kTargetPos),
	_physics(std::make_shared<Physics>()),
	_skydomeHandle(-1),
	_arena(std::make_shared<Arena>()),
	_billboardManager(std::make_shared<BillboardManager>())
{
	//_titleFontHandle = CreateFontToHandle(kFontName.c_str(), kTitleFontSize, kFontThickness,
	//	DX_FONTTYPE_ANTIALIASING_EDGE);
	//assert(_titleFontHandle >= 0 && "フォントの作成に失敗");
	_nextSceneFontHandle = CreateFontToHandle(kFontName.c_str(), kNextSceneFontSize, kFontThickness,
		DX_FONTTYPE_ANTIALIASING_EDGE);
	assert(_nextSceneFontHandle >= 0 && "フォントの作成に失敗");


	//_titleGraphHandle = LoadGraph(L"data/img/background/Title.png");
	//assert(_titleGraphHandle >= 0);
}

SceneTitle::~SceneTitle()
{
	// フォント解放
	if (_titleFontHandle != -1) {
		DeleteFontToHandle(_titleFontHandle);
	}
	if (_nextSceneFontHandle != -1) {
		DeleteFontToHandle(_nextSceneFontHandle);
	}

	if (_skydomeHandle != -1) {
		MV1DeleteModel(_skydomeHandle);
	}
	
	//DeleteGraph();
}

void SceneTitle::Init()
{
	_titleImageHandle = LoadGraph(L"data/graph/background/Title.png");
	assert(_titleImageHandle >= 0);

	_skydomeHandle = MV1LoadModel(L"data/skydome/Sky_Twilight01.mv1");
	assert(_skydomeHandle >= 0);
	MV1SetPosition(_skydomeHandle, Position3(0, 0, 0));
	MV1SetScale(_skydomeHandle, Vector3(1, 1, 1) * 4.0f);

	_arena->Init(_physics);
	_billboardManager->Init();

	// カメラの位置、描画距離、画角を更新
	SetCameraPositionAndTarget_UpVecY(kCameraPos, _targetPos);
	SetCameraNearFar(kNear, kFar);
	SetupCamera_Perspective(kViewAngle);

	SoundManager::GetInstance().PlaySoundType(BGMType::Title, true, false);
}

void SceneTitle::Update()
{
	(this->*_nowUpdateState)();
}

void SceneTitle::Draw()
{
	(this->*_nowDrawState)();
}

void SceneTitle::FadeinUpdate()
{
	_frame--;
	if (_frame <= 0) {
		_nowUpdateState = &SceneTitle::NormalUpdate;
		_nowDrawState = &SceneTitle::NormalDraw;
	}


	// フェードイン時は点滅を行わない

	// カメラの注視点を更新
	_cameraAngleY += kCameraRotationSpeed;
	_targetPos.x = cosf(_cameraAngleY) * kCameraTargetRadius;
	_targetPos.z = sinf(_cameraAngleY) * kCameraTargetRadius;
	SetCameraPositionAndTarget_UpVecY(kCameraPos, _targetPos);

	_billboardManager->Update();
}

void SceneTitle::NormalUpdate()
{
	// 決定を押したら
	if (Input::GetInstance().IsTrigger("Title:ChangeGameScene")) {
		_nextSceneName = NextSceneName::GamePlay;
		_nowUpdateState = &SceneTitle::FadeoutUpdate;
		_nowDrawState = &SceneTitle::FadeDraw;
		_frame = 0;
		// FadeUpdateに移る前の処理を行う
		_isNextSceneTextActive = false;
		_nextSceneTextTickFrame = 0;

		SoundManager::GetInstance().PlaySoundType(SEType::Enter2);
	}
	else if (Input::GetInstance().IsTrigger("Title:ChangeInstructionScene")) {
		_nextSceneName = NextSceneName::Instruction;
		_nowUpdateState = &SceneTitle::FadeoutUpdate;
		_nowDrawState = &SceneTitle::FadeDraw;
		_frame = 0;
		// FadeUpdateに移る前の処理を行う
		_isNextSceneTextActive = false;
		_nextSceneTextTickFrame = 0;

		SoundManager::GetInstance().PlaySoundType(SEType::Enter1);
	}


	_nextSceneTextTickFrame++;

	// 文字描画状態切り替え
	// 表示されている　　かつ　出現時間を超えている
	// または
	// 表示されていない　かつ　消滅時間を超えているなら
	if ((_isNextSceneTextActive &&
		_nextSceneTextTickFrame >= kNextSceneFontThicknessIn) ||
		(!_isNextSceneTextActive && 
		_nextSceneTextTickFrame >= kNextSceneFontThicknessOut)) {

		// 時間をリセットし描画状態を反転させる
		_nextSceneTextTickFrame = 0;
		_isNextSceneTextActive = !_isNextSceneTextActive;
	}

	// カメラの注視点を更新
	_cameraAngleY += kCameraRotationSpeed;
	_targetPos.x = cosf(_cameraAngleY) * kCameraTargetRadius;
	_targetPos.z = sinf(_cameraAngleY) * kCameraTargetRadius;
	SetCameraPositionAndTarget_UpVecY(kCameraPos, _targetPos);

	_billboardManager->Update();
}

void SceneTitle::FadeoutUpdate()
{
	_frame++;

	if (_frame >= Statistics::kFadeInterval) {
		FadeDraw();
		if (_nextSceneName == NextSceneName::GamePlay) {
			_nextScene = std::make_shared<SceneGamePlay>();
		}
		else if (_nextSceneName == NextSceneName::Instruction) {
			_nextScene = std::make_shared<SceneOperationInstruction>();
		}
		else {
			assert(false && "次のシーンが不明");
		}

		if (_nextScene == nullptr) {
			assert(false && "次のシーンが不明");
		}
		SceneController::GetInstance().ChangeScene(_nextScene);
		return;  // 自分が死んでいるのでもし
		// 余計な処理が入っているとまずいのでreturn;
	}


	_nextSceneTextTickFrame++;

	// 文字描画状態切り替え
	// 点滅切り替え時間を超えているなら
	if (_nextSceneTextTickFrame >= kNextSceneFontThicknessChange) {
		// 時間をリセットし描画状態を反転させる
		_nextSceneTextTickFrame = 0;
		_isNextSceneTextActive = !_isNextSceneTextActive;
	}

	// カメラの注視点を更新
	_cameraAngleY += kCameraRotationSpeed;
	_targetPos.x = cosf(_cameraAngleY) * kCameraTargetRadius;
	_targetPos.z = sinf(_cameraAngleY) * kCameraTargetRadius;
	SetCameraPositionAndTarget_UpVecY(kCameraPos, _targetPos);

	_billboardManager->Update();
}

void SceneTitle::FadeDraw()
{
	MV1DrawModel(_skydomeHandle);
	_arena->Draw();
	_billboardManager->Draw();

	// タイトル文字、背景描画
	DrawTitleGraph();

	// タイトル文字を描画する
	DrawTitleString();


#ifdef _DEBUG
	DrawFormatString(0, 0, 0xffffff, L"Scene Title");
#endif


	// フェードイン/アウトの処理
	// フェード割合の計算(0.0-1.0)
	float rate = static_cast<float>(_frame) / static_cast<float>(Statistics::kFadeInterval);
	SetDrawBlendMode(DX_BLENDMODE_MULA, static_cast<int>(255 * rate));
	DrawBox(0, 0, Statistics::kScreenWidth, Statistics::kScreenHeight, 0x000000, true);
	// BlendModeを使った後はNOBLENDにしておくことを忘れず
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void SceneTitle::NormalDraw()
{
	MV1DrawModel(_skydomeHandle);
	_arena->Draw();
	_billboardManager->Draw();

	// タイトル文字、背景描画
	DrawTitleGraph();

	// タイトル文字を描画する
	DrawTitleString();


#ifdef _DEBUG
	DrawFormatString(0, 0, 0xffffff, L"Scene Title");
#endif
}

void SceneTitle::DrawTitleGraph()
{
	// タイトル文字描画
	int titleWidth, titleHeight;
	GetGraphSize(_titleImageHandle, &titleWidth, &titleHeight);
	Position2 pos = Position2(Statistics::kScreenCenterWidth,
		Statistics::kScreenCenterHeight * 0.4f);
	DrawRectRotaGraph(static_cast<int>(pos.x), static_cast<int>(pos.y),
		0, 0,
		titleWidth, titleHeight,
		0.4, 0.0,
		_titleImageHandle, true,
		false, false);
}

void SceneTitle::DrawTitleString()
{
	// 文字描画

	//// 文字の中央までの長さをはかり
	//// 描画開始位置を求め描画する
	//int titleTextWidth = GetDrawStringWidthToHandle(
	//	kTitleText.c_str(),
	//	kTitleText.length(),
	//	_titleFontHandle);
	//int titleDrawX = (Statistics::kScreenWidth - titleTextWidth) * 0.5f;
	//DrawStringToHandle(
	//	titleDrawX, kTitleTextY,
	//	kTitleText.c_str(), kTextColor,
	//	_titleFontHandle);

	// 消滅中は描画を行わない
	if (!_isNextSceneTextActive) return;

	// 最後の入力に応じて文字を変える
	std::wstring drawString = kPadNextSceneText;
	if (Input::GetInstance().GetLastInputType() == Input::PeripheralType::keybd) {
		drawString = kKeybdNextSceneText;
	}

	// 同様に描画する
	int nextSceneTextWidth = GetDrawStringWidthToHandle(
		drawString.c_str(),
		static_cast<int>(drawString.length()),
		_nextSceneFontHandle);
	int NextSceneDrawX = static_cast<int>((Statistics::kScreenWidth - nextSceneTextWidth) * 0.5f);
	DrawStringToHandle(
		NextSceneDrawX, kNextSceneTextY,
		drawString.c_str(), kTextColor,
		_nextSceneFontHandle);
}
