#include "EnemyManager.h"
#include "EnemyFactory.h"
#include "EnemyBase.h"
#include "WaveData.h"
#include "Calculation.h"
#include "Player.h"
#include "Physics.h"
#include <algorithm>
#include <DxLib.h>

EnemyManager::EnemyManager() :
    _enemies(),
    _player(),
    _physics()
{
    // 処理なし
}

EnemyManager::~EnemyManager()
{
    // 処理なし
}

void EnemyManager::Init(std::weak_ptr<Player> player, std::weak_ptr<Physics> physics)
{
    _player = player;
    _physics = physics;
}

void EnemyManager::Update()
{
    for (auto& enemy : _enemies)
    {
        enemy->Update();
    }
}

void EnemyManager::Draw()
{
    for (const auto& enemy : _enemies)
    {
        enemy->Draw();
    }
}

void EnemyManager::SpawnEnemies(const std::vector<SpawnInfo>& spawnInfoList)
{
    // 死亡済みの敵をリストから削除
    CleanupDefeatedEnemies();

    for (const auto& info : spawnInfoList)
    {
        for (int i = 0; i < info.count; ++i)
        {
            // spawnRadius内にランダムな位置を計算
            float angle = Calc::ToRadian(static_cast<float>(GetRand(360)));
            float radius = static_cast<float>(GetRand(static_cast<int>(info.spawnRadius)));
            Position3 spawnPos = info.basePosition + Vector3(cos(angle) * radius, 0.0f, sin
            (angle) *radius);

            // Factoryを使って敵を生成し、リストに追加
            auto newEnemy = EnemyFactory::CreateAndRegister(info.type, spawnPos, _player,
                _physics);
            _enemies.emplace_back(newEnemy);
        }
    }
}

bool EnemyManager::AreAllEnemiesDefeated() const
{
    for (const auto& enemy : _enemies)
    {
        if (enemy->GetState() != EnemyBase::State::Dead)
        {
            return false; // 1体でも生きていればfalse
        }
    }
    return true; // 全てDead状態
}

const std::vector<std::shared_ptr<EnemyBase>>& EnemyManager::GetEnemies() const
{
    return _enemies;
}

std::weak_ptr<EnemyBase> EnemyManager::GetNearestEnemy(Position3 pos, EnemyType type, bool isDead)
{
    // 返す敵のタイプ
    EnemyType retType = type;
    std::weak_ptr<EnemyBase> ret;
    if (retType == EnemyType::TypeNum) retType = EnemyType::None;
    float nearestDist = FLT_MAX;

    for (const std::shared_ptr<EnemyBase> enemy : _enemies) {
        // 死んでいる敵を除外するかつ
        // 敵が死んでいるなら
        if (!isDead && !enemy->IsAlive()) continue;
        // タイプがNoneもしくは
        // タイプが一致しているなら
        if (retType == EnemyType::None ||
            enemy->GetType() == retType) {
            // 距離を測り既存値以内なら
            float dist = (enemy->GetPos() - pos).Magnitude();
            if (dist < nearestDist) {
                // 情報を更新する
                nearestDist = dist;
                ret = enemy;
            }
        }
    }

    return ret;
}

void EnemyManager::CleanupDefeatedEnemies()
{
    // StateがDeadの敵をvectorの末尾に集めてから削除する
    _enemies.erase(
        std::remove_if(_enemies.begin(), _enemies.end(),
            [](const std::shared_ptr<EnemyBase>& enemy) {
                return (enemy->GetState() == EnemyBase::State::Dead);
            }),
        _enemies.end()
    );
}
