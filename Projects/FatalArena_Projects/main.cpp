#include <DxLib.h>

#include "Application.h"

using namespace std;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	Application& app = Application::GetInstance();

	// アプリケーションの初期化
	if (!app.Init())
	{
		return -1;
	}

	// メインループ
	app.Run();

	// 後処理
	app.Terminate();

	return 0;
}