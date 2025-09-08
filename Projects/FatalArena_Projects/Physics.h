#pragma once
#include <memory>
#include <list>

class Collider;

/// <summary>
/// 物理挙動を司る
/// </summary>
class Physics final {
public:
	/// <summary>
	/// オブジェクト登録
	/// </summary>
	void Entry(std::shared_ptr<Collider> collider);

	/// <summary>
	/// オブジェクト登録解除
	/// </summary>
	void Release(std::shared_ptr<Collider> collider);

	void Update();

private:

	// OnCollideの遅延通知のためのデータ
	struct OnCollideInfo
	{
		std::shared_ptr<Collider> owner;
		std::shared_ptr<Collider> colider;
	};

	// 登録されたColliderのリスト
	std::list<std::shared_ptr<Collider>> _colliders;

	std::list<OnCollideInfo> CheckCollide() const;

	/// <summary>
	/// 当たっているかどうかだけ判定
	/// </summary>
	bool IsCollide(const std::shared_ptr<Collider> objA, const std::shared_ptr<Collider> objB) const;

	/// <summary>
	/// 第一引数のColliderを動かないものとして、
	/// 第二引数に入ったColliderの位置を補正する
	/// 第三引数にtrueが入っていた場合はそれらを無視し両方を押し戻す
	/// </summary>
	/// <param name="primary">動かないCollider</param>
	/// <param name="secondary">補正を行うCollider</param>
	/// <param name="isMutualPushback">両方を押し戻すか</param>
	void FixNextPosition(std::shared_ptr<Collider> primary, std::shared_ptr<Collider> secondary, bool isMutualPushback) const;
	/// <summary>
	/// 位置決定
	/// </summary>
	void FixPosition();
};

