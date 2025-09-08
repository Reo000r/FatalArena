#pragma once
#include <string>

namespace Statistics
{
	constexpr int kScreenWidth  = 1920;
	constexpr int kScreenHeight = 1080;
	//constexpr int kScreenWidth  = 1280;
	//constexpr int kScreenHeight = 720;
	
	constexpr float kScreenCenterWidth  = kScreenWidth * 0.5f;
	constexpr float kScreenCenterHeight = kScreenHeight * 0.5f;

	constexpr int kFadeInterval = 30;

	//const std::wstring kDefaultFontName = L"BIZ UDP明朝 Medium";
	const std::wstring kDefaultFontName = L"Impact";
}