#pragma once
#include "Weapon.h"

class WeaponPlayer final :  public Weapon
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	WeaponPlayer();

	/// <summary>
	/// 衝突したときに呼ばれる
	/// </summary>
	/// <param name="colider"></param>
	void OnCollide(const std::weak_ptr<Collider> collider) override;
};

