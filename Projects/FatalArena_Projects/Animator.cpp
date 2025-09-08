#include "Animator.h"

#include <DxLib.h>
#include <cassert>

namespace {
	// アニメーションのブレンド時間(frame)
	constexpr float kAnimBlendFrame = 15.0f;
	// 全てにかかるアニメーションの再生速度(60fなら1.0、30fなら0.5が等速)
	constexpr float kAnimSpeed = 1.3f;
}

Animator::Animator() :
	_model(-1),
	_animDataList(),
	_currentAnimName(L""),
	_prevAnimName(L""),
	_blendRate(1.0f) // 最初はブレンドしていないので1.0
{
}

Animator::~Animator()
{
	MV1DeleteModel(_model);
}

void Animator::Init(int model)
{
	assert(model >= 0 && "モデルハンドルが正しくない");
	_model = model;
}

void Animator::Update()
{
	// ブレンド処理が進行中か、
	// そうでなければ現在のアニメーションを更新
	UpdateAnimBlendRate();
}

void Animator::SetStartAnim(const std::wstring animName)
{
	// 最初のアニメーションを現在のものとして設定
	_currentAnimName = animName;
	AnimData& currentAnim = FindAnimData(_currentAnimName);
	AttachAnim(_currentAnimName, currentAnim.isLoop);

	// ブレンドは不要なので、ウェイトを100%にする
	_blendRate = 1.0f;
	MV1SetAttachAnimBlendRate(
		_model, 
		FindAnimData(_currentAnimName).attachNo, 
		_blendRate);
}

void Animator::SetAnimData(const std::wstring animName, const float animSpeed, const bool isLoop, float inputAcceptanceStartRatio, float inputAcceptanceEndRatio)
{
	// すでに同じアニメーションが登録されていないか確認
	for (const auto& anim : _animDataList) {
		if (animName == anim.animName) {
			assert(false && "同一のアニメーションを登録しようとしている");
			return;
		}
	}
	AnimData animData;
	animData.animIndex = MV1GetAnimIndex(_model, animName.c_str());
	assert(animData.animIndex >= 0 && "存在しないアニメーションを登録しようとしている");
	animData.attachNo = -1;	// 実際に使う際に更新する
	animData.animName = animName;
	animData.animSpeed = animSpeed * kAnimSpeed;
	animData.frame = 0.0f;
	animData.totalFrame = MV1GetAnimTotalTime(_model, animData.animIndex);
	animData.isLoop = isLoop;
	animData.isEnd = false;
	// 比率をフレーム値に変換
	animData.inputAcceptanceStartFrame = animData.totalFrame * inputAcceptanceStartRatio;
	animData.inputAcceptanceEndFrame = animData.totalFrame * inputAcceptanceEndRatio;
	_animDataList.emplace_front(animData);
}

void Animator::AttachAnim(const std::wstring animName, const bool isLoop)
{
	// アニメーション名が空なら何もしない
	if (animName.empty()) return;

	AnimData& animData = FindAnimData(animName);

	// すでにアタッチ済みなら何もしない
	if (animData.attachNo >= 0) return;

	// モデルにアニメーションをアタッチ
	animData.attachNo = MV1AttachAnim(_model, animData.animIndex, -1, false);
	assert(animData.attachNo >= 0 && "アニメーションのアタッチ失敗");
	animData.frame = 0.0f;
	animData.isLoop = isLoop;
	animData.isEnd = false;
	// 再生時間をリセット
	MV1SetAttachAnimTime(_model, animData.attachNo, 0.0f);
}

void Animator::UpdateAnim(AnimData& data)
{
	// アニメーションがアタッチされていない場合return
	if (data.attachNo == -1) return;
	// アニメーションを進める
	data.frame += data.animSpeed;
	// 現在再生中のアニメーションの総時間を取得する
	const float totalTime = data.totalFrame;
	
	// アニメーションの設定によってループさせるか最後のフレームで止めるかを判定
	if (data.isLoop)
	{
		// アニメーションをループさせる
		while (data.frame > totalTime)
		{
			data.frame -= totalTime;
		}
	}
	else
	{
		// 最後のフレームで停止させる
		if (data.frame > totalTime)
		{
			data.frame = totalTime;
			data.isEnd = true;
		}
	}

	// 進行させたアニメーションをモデルに適用する
	MV1SetAttachAnimTime(_model, data.attachNo, data.frame);
}

void Animator::UpdateAnimBlendRate()
{
	// 現在のアニメーションを進める
	if (!_currentAnimName.empty()) {
		UpdateAnim(FindAnimData(_currentAnimName));
	}

	// ブレンド中かどうか
	if (_blendRate < 1.0f) {
		// 古いアニメーションのフレームは更新せず
		// 影響度だけを下げる

		// ブレンド率を更新
		if (kAnimBlendFrame) _blendRate += 1.0f / kAnimBlendFrame;

		// ブレンドが完了した場合の処理
		if (_blendRate >= 1.0f) {
			_blendRate = 1.0f;

			// 古いアニメーションは完全に不要になったのでデタッチ
			if (!_prevAnimName.empty()) {
				AnimData& prevAnim = FindAnimData(_prevAnimName);
				if (prevAnim.attachNo != -1) {
					MV1DetachAnim(_model, prevAnim.attachNo);
					prevAnim.attachNo = -1;
				}
				// 前のアニメーション名をクリアしてブレンド処理を終了
				_prevAnimName.clear();
			}
		}
	}

	// モデルにブレンド率を適用
	if (!_currentAnimName.empty()) {
		MV1SetAttachAnimBlendRate(_model, FindAnimData(_currentAnimName).attachNo, _blendRate);
	}
	if (!_prevAnimName.empty()) {
		MV1SetAttachAnimBlendRate(_model, FindAnimData(_prevAnimName).attachNo, 1.0f - _blendRate);
	}
}

void Animator::ChangeAnim(const std::wstring animName, bool isLoop)
{
	// 既に再生中、または遷移しようとしているアニメーションならreturn
	if (animName == _currentAnimName) return;

	// もし現在ブレンド中なら、
	// そのブレンド元のアニメーションは不要になるのでデタッチ
	if (!_prevAnimName.empty()) {
		AnimData& prevAnim = FindAnimData(_prevAnimName);
		if (prevAnim.attachNo != -1) {
			MV1DetachAnim(_model, prevAnim.attachNo);
			prevAnim.attachNo = -1;
		}
	}

	// 今まで再生していたアニメーションを前のアニメーションにし、
	// 新しいアニメーションを現在のアニメーションにする
	_prevAnimName = _currentAnimName;
	_currentAnimName = animName;
	// 新しいアニメーションをアタッチ
	AttachAnim(_currentAnimName, isLoop);
	_blendRate = 0.0f;

	// アタッチしたばかりのアニメーションに少しだけ影響力を持たせる
	UpdateAnimBlendRate();
}

Animator::AnimData& Animator::FindAnimData(const std::wstring animName)
{
	// アニメーション名が空の場合はリストの先頭をダミーとして返す
	if (animName.empty()) {
		return _animDataList.front();
	}

	// 検索
	for (auto& data : _animDataList) {
		if (animName == data.animName)  return data;
	}

	assert(false && "指定の名前のアニメーションが登録されていなかった");
	return _animDataList.front();
}

float Animator::GetCurrentAnimFrame()
{
	if (_currentAnimName.empty()) return 0.0f;
	return FindAnimData(_currentAnimName).frame;
}
