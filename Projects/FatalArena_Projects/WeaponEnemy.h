#pragma once
#include "Weapon.h"
class WeaponEnemy final : public Weapon
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	WeaponEnemy();

	/// <summary>
	/// 衝突したときに呼ばれる
	/// </summary>
	/// <param name="colider"></param>
	void OnCollide(const std::weak_ptr<Collider> collider) override;
};

