#pragma once
#include "Collider.h"

class Arena final : public Collider
{
public:
	Arena();
	~Arena();

	void Init(std::weak_ptr<Physics> physics);
	void Draw();

	/// <summary>
	/// 衝突したときに呼ばれる
	/// </summary>
	/// <param name="colider"></param>
	void OnCollide(const std::weak_ptr<Collider> collider) override;

	/// <summary>
	/// アリーナの半径を返す
	/// (当たり判定の内側のサイズ)
	/// </summary>
	/// <returns></returns>
	static float GetArenaRadius();

private:

	int _modelHandle;
};

