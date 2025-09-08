#include "WeaponEnemy.h"
#include "Player.h"
#include "EnemyBase.h"

WeaponEnemy::WeaponEnemy() :
	Weapon(PhysicsData::GameObjectTag::EnemyAttack)
{
    // 処理なし
}

void WeaponEnemy::OnCollide(const std::weak_ptr<Collider> collider)
{
    auto other = collider.lock();
    auto owner = _owner.lock();

    // 相手や所有者が不明な場合は何もしない
    if (collider.expired() || owner == nullptr) {
        return;
    }

    // プレイヤーでないなら攻撃しない
    if (other->GetTag() != PhysicsData::GameObjectTag::Player) {
        return;
    }

    auto player = std::static_pointer_cast<Player>(other);
    auto enemy = std::static_pointer_cast<EnemyBase>(owner);
    
    // 相手にダメージ処理を依頼する
    player->TakeDamage(enemy->GetAttackPower(), owner);
    _isHit = true;

    // 所有者がPlayerでない場合は
    // 一度ダメージを与えたら、連続ヒットを防ぐため当たり判定を無効にする
    SetCollisionState(false);
}
