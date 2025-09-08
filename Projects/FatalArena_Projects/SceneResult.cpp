#include "SceneResult.h"
#include "SceneTitle.h"		// 遷移先のシーン
#include "SceneGamePlay.h"  // 遷移先のシーン
#include "SceneBase.h"
#include "SceneController.h"
#include "ResultDisplay.h"
#include "GameManager.h"
#include "SoundManager.h"

#include "Input.h"
#include "Statistics.h"

#include <memory>
#include <DxLib.h>
#include <cassert>

SceneResult::SceneResult() :
	_frame(Statistics::kFadeInterval),
	_nextSceneName(NextSceneName::Title),
	_nextScene(nullptr),
	_nowUpdateState(&SceneResult::FadeinUpdate),
	_nowDrawState(&SceneResult::FadeDraw),
	_resultDisplay(std::make_unique<ResultDisplay>())
{
}

SceneResult::~SceneResult()
{
}

void SceneResult::Init()
{
	// リザルト表示の前にタイムボーナスを計算させる
	GameManager::GetInstance().CalculateTimeBonus();

	_resultDisplay->Init();

	SoundManager::GetInstance().PlaySoundType(BGMType::Result, true, false);
}

void SceneResult::Update()
{
	(this->*_nowUpdateState)();
}

void SceneResult::Draw()
{
	(this->*_nowDrawState)();
}

void SceneResult::FadeinUpdate()
{
	_frame--;
	if (_frame <= 0) {
		_nowUpdateState = &SceneResult::NormalUpdate;
		_nowDrawState = &SceneResult::NormalDraw;
	}
}

void SceneResult::NormalUpdate()
{
	_resultDisplay->NormalUpdate();

	//　リザルトアニメーションが終わっている状態で
	if (_resultDisplay->IsAnimationFinished()) {
		// 決定を押したら
		if (Input::GetInstance().IsTrigger("Result:ChangeGameScene")) {
			_nextSceneName = NextSceneName::GamePlay;
			_nowUpdateState = &SceneResult::FadeoutUpdate;
			_nowDrawState = &SceneResult::FadeDraw;
			_frame = 0;
			SoundManager::GetInstance().PlaySoundType(SEType::Enter2);
		}
		else if (Input::GetInstance().IsTrigger("Result:ChangeTitleScene")) {
			_nextSceneName = NextSceneName::Title;
			_nowUpdateState = &SceneResult::FadeoutUpdate;
			_nowDrawState = &SceneResult::FadeDraw;
			_frame = 0;
			SoundManager::GetInstance().PlaySoundType(SEType::Enter1);
		}
	}
}

void SceneResult::FadeoutUpdate()
{
	_frame++;

	if (_frame >= Statistics::kFadeInterval) {
		if (_nextSceneName == NextSceneName::GamePlay) {
			_nextScene = std::make_shared<SceneGamePlay>();
		}
		else if (_nextSceneName == NextSceneName::Title) {
			_nextScene = std::make_shared<SceneTitle>();
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


	// フェードアウト時の更新処理を呼ぶ
	_resultDisplay->FadeoutUpdate();
}

void SceneResult::FadeDraw()
{
	_resultDisplay->Draw();

#ifdef _DEBUG
	//DrawFormatString(0, 0, 0xffffff, L"Scene Result");
#endif

	// フェードイン/アウトの処理
	// フェード割合の計算(0.0-1.0)
	float rate = static_cast<float>(_frame) / static_cast<float>(Statistics::kFadeInterval);
	SetDrawBlendMode(DX_BLENDMODE_MULA, static_cast<int>(255 * rate));
	DrawBox(0, 0, Statistics::kScreenWidth, Statistics::kScreenHeight, 0x000000, true);
	// BlendModeを使った後はNOBLENDにしておくことを忘れず
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void SceneResult::NormalDraw()
{
	_resultDisplay->Draw();

#ifdef _DEBUG
	//DrawFormatString(0, 0, 0xffffff, L"Scene Result");
#endif
}
