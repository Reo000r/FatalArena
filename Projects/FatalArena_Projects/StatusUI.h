#pragma once
#include "Vector3.h"
#include <memory>
#include <map>

class Player;
class WaveManager;
class EnemyManager;
class EnemyBase;

class StatusUI
{
public:
	StatusUI();
	~StatusUI();

	void Init(std::weak_ptr<Player> player, std::weak_ptr<WaveManager> waveManager, std::weak_ptr<EnemyManager> enemyManager) ;
	void Update();
	void Draw();

private:

	void DrawPlayerHp();
	void DrawPlayerStamina();
	void DrawEnemyHp(std::shared_ptr<EnemyBase> enemy);
	void DrawScore();

	std::weak_ptr<Player> _player;
	std::weak_ptr<WaveManager> _waveManager;
	std::weak_ptr<EnemyManager> _enemyManager;

	int _scoreFontHandle;
};

