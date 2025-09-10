#pragma once
class WaveManager;

/// <summary>
/// ウェーブの開始を通知する表示を管理するクラス
/// </summary>
class WaveAnnouncer
{
public:
	WaveAnnouncer();
	~WaveAnnouncer();

	void Update();
	void Draw();

	// 表示を開始する
	void Start(int currentWave, int maxWave);

	// 表示が完了したか
	bool IsFinished() const;

private:
	bool _isDisplaying; // 表示中かどうかのフラグ
	int _displayTimer;  // 表示時間を管理するタイマー
	int _currentWave;   // 表示するウェーブ番号
	int _maxWave;		// 最大ウェーブ数

	int _fontHandle;    // 表示に使用するフォントハンドル
};
