#include "ItemBase.h"
#include "Rigidbody.h"
#include "ColliderDataSphere.h"
#include "Calculation.h"
#include "Player.h"

#include <DxLib.h>
#include <cassert>

ItemBase::ItemBase(BuffData data, int modelHandle, 
	std::weak_ptr<PlayerBuffManager> manager) :
	Collider(PhysicsData::Priority::Static,
		PhysicsData::GameObjectTag::Item,
		PhysicsData::ColliderKind::Sphere,
		true, false),
	_nowUpdateState(&ItemBase::UpdateSpawning),
	_modelHandle(modelHandle),
	_transOffset(),
	_scale(),
	_rotAngle(),
	_modelOffset(),
	_depthY(0.0f),
	_modelRotSpeed(0.0f),
	_playerBuffManager(manager),
	_data(data),
	_isAlive(true),
	_animFrame(0),
	_totalAnimFrame(0)
{
	assert(_modelHandle >= 0 && "モデルハンドルが正しくない");

	rigidbody->Init(false);
}

ItemBase::~ItemBase()
{
	// モデル解放
	if (_modelHandle != -1) MV1DeleteModel(_modelHandle);
}

void ItemBase::Init(float colRad, Vector3 transOffset, Vector3 scale, Vector3 angle)
{
	_scale = scale;

	_transOffset = transOffset;
	_rotAngle = angle;
	_scale = scale;

	// 当たり判定データ設定
	SphereColliderDesc desc;
	desc.radius = colRad;
	colliderData = CreateColliderData(
		desc,	// 種別
		true,	// isTrigger
		false	// isCollision
	);

	// 攻撃判定、敵、他のアイテムとは当たり判定を行わない
	colliderData->AddThroughTag(PhysicsData::GameObjectTag::PlayerAttack);
	colliderData->AddThroughTag(PhysicsData::GameObjectTag::EnemyAttack);
	colliderData->AddThroughTag(PhysicsData::GameObjectTag::Enemy);
	colliderData->AddThroughTag(PhysicsData::GameObjectTag::Item);

	// 最初は当たり判定を行わない
	SetCollisionState(false);
}

void ItemBase::Update()
{
	// ステートに応じた更新を行う
	(this->*_nowUpdateState)();
}

void ItemBase::Draw()
{
	// 描画
	MV1DrawModel(_modelHandle);
}

void ItemBase::Spawn(Position3 pos, float depthY, int totalAnimFrame, float modelRotSpeed)
{
	_nowUpdateState = &ItemBase::UpdateSpawning;
	_spawnPos = pos;
	_depthY = depthY;
	_modelRotSpeed = modelRotSpeed;

	// 生成位置に移動する
	Position3 spawnPos = _spawnPos;
	spawnPos.y += _depthY;
	rigidbody->SetPos(spawnPos);
	MV1SetPosition(_modelHandle, spawnPos);

	_totalAnimFrame = totalAnimFrame;
	_animFrame = _totalAnimFrame;	// 比率を1->0に遷移させたいため
}

void ItemBase::Destroy()
{
}

void ItemBase::SetCollisionState(bool isCollision)
{
	colliderData->isCollision = isCollision;
}

bool ItemBase::GetCollisionState()
{
	return colliderData->isCollision;
}

void ItemBase::UpdateSpawning()
{
	_animFrame--;	// 比率を1->0に遷移させたいため

	float ratio = static_cast<float>(_animFrame) / _totalAnimFrame;
	float depthY = _depthY * (ratio);
	Position3 pos = _spawnPos + Position3(0, depthY, 0);

	SetMatrix(pos, _modelRotSpeed);

	// 条件を満たしたらステート変更
	if (_animFrame <= 0) {
		_nowUpdateState = &ItemBase::UpdateIdle;
		_animFrame = 0;
		// 当たり判定をつける
		SetCollisionState(true);
	}
}

void ItemBase::UpdateIdle()
{
	SetMatrix(_spawnPos, _modelRotSpeed);
}

void ItemBase::UpdateDestroying()
{
	_animFrame++;	// 比率を0->1に遷移させたいため

	float ratio = static_cast<float>(_animFrame) / _totalAnimFrame;
	float depthY = _depthY * (ratio);
	Position3 pos = _spawnPos + Position3(0, depthY, 0);

	SetMatrix(pos, _modelRotSpeed);

	// 条件を満たしたら
	if (_animFrame >= _totalAnimFrame &&
		_isAlive) {
		ReleasePhysics();
		_isAlive = false;
	}
}

void ItemBase::SetMatrix(Position3 pos, float rotSpeed)
{
	_rotAngle.y += rotSpeed;
	// 各オフセットの行列を生成
	Matrix4x4 scaleMatrix = MatGetScale(_scale);
	Matrix4x4 rotY = MatRotateY(_rotAngle.y);
	Matrix4x4 rotX = MatRotateX(_rotAngle.x);
	Matrix4x4 rotZ = MatRotateZ(_rotAngle.z);
	Matrix4x4 rotationMatrix = MatMultiple(MatMultiple(rotZ, rotX), rotY);
	Matrix4x4 translationMatrix = MatTranslate(_transOffset);
	Matrix4x4 positionMatrix = MatTranslate(pos + _modelOffset);

	// 行列に対して補正値を合成する
	// 平行移動 -> 回転 -> 拡縮の順
	Matrix4x4 worldMatrix = MatMultiple(scaleMatrix, MatMultiple(rotationMatrix,
		MatMultiple(translationMatrix, positionMatrix)));

	// モデルに最終的なワールド行列を適用
	MV1SetMatrix(_modelHandle, worldMatrix);

	// Rigidbodyの位置と当たり判定の向きを更新
	// (回転とスケールが適用される前の行列から計算)
	auto m1 = MatMultiple(translationMatrix, positionMatrix);
	Position3 modelWorldPos = Vector3(
		m1.m[3][0],
		m1.m[3][1],
		m1.m[3][2]
	) - _modelOffset;
	rigidbody->SetPos(modelWorldPos);
}

void ItemBase::OnCollide(const std::weak_ptr<Collider> collider)
{
	// 相手が不明な場合は何もしない
	if (collider.expired()) {
		return;
	}

    auto other = collider.lock();
    
    // プレイヤーでないなら何もしない
    if (other->GetTag() != PhysicsData::GameObjectTag::Player) {
        return;
    }

	// 触れたことを伝える
	_playerBuffManager.lock()->AttachBuff(_data);

	// 取得SE
	PlayGetSE();

	// 消滅中にする
	_nowUpdateState = &ItemBase::UpdateDestroying;
	// 当たり判定を切る
	SetCollisionState(false);
}
