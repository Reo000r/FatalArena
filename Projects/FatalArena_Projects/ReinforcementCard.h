#pragma once
#include "Vector3.h"

/// <summary>
/// カード種別
/// </summary>
enum class CardType : int {
	MaxHealth,
	Stamina,
	Strength,
	TypeNum,
};

class ReinforcementCard
{
public:
	ReinforcementCard();
	~ReinforcementCard();

	void Init(int cardHandle, int iconHandle, Position3 centerPos, 
		float cardScaleOffset, float iconScaleOffset);
	void Update();
	void Draw();

	Position3 GetCenterPos() const { return _centerPos; }

private:

	int _cardHandle;
	int _iconHandle;
	Position3 _centerPos;
	float _cardScaleOffset;
	float _iconScaleOffset;

	int _cardWidth;		// カード幅
	int _cardHeight;	// カード高さ
	int _iconWidth;		// アイコン幅
	int _iconHeight;	// アイコン高さ

};

