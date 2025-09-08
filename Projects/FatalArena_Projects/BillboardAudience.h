#pragma once
#include "Vector3.h"

/// <summary>
/// 観客ビルボード
/// </summary>
class BillboardAudience
{
public:

	BillboardAudience(int modelHandle, float defaultProgress);
	~BillboardAudience();

	void SetPos(Position3 pos);
	void Update();
	void Draw();

private:

	int _modelHandle;
	Position3 _pos;

	int _animFrame;
};

