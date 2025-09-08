#pragma once
#include "Geometry.h"
#include "Collider.h"
#include "PlayerBuffManager.h"
#include <list>
#include <memory>

class PlayerBuffManager;

/// <summary>
/// アイテムの基底クラス
/// </summary>
class ItemBase abstract : public Collider
{
public:
	ItemBase(BuffData data, int modelHandle, 
		std::weak_ptr<PlayerBuffManager> manager);
	virtual ~ItemBase();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="colRad">当たり判定半径</param>
	/// <param name="transOffset">位置補正</param>
	/// <param name="scale">拡縮補正</param>
	/// <param name="angle">角度補正</param>
	void Init(float colRad,
		Vector3 transOffset = Vector3(),
		Vector3 scale = Vector3(1, 1, 1),
		Vector3 angle = Vector3());

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 衝突したときに呼ばれる
	/// プレイヤーが触れた場合は通知を送る
	/// </summary>
	/// <param name="colider"></param>
	void OnCollide(const std::weak_ptr<Collider> collider) override;

	/// <summary>
	/// 生成を開始する
	/// </summary>
	/// <param name="pos">生成位置</param>
	/// <param name="depthY">生成深度</param>
	/// <param name="totalAnimFrame">生成位置に向かうまでの時間(フレーム)</param>
	/// <param name="modelRotSpeed">1f当たりのモデルの回転量</param>
	void Spawn(Position3 pos, float depthY, int totalAnimFrame, float modelRotSpeed);
	
	/// <summary>
	/// 消滅処理を開始する
	/// </summary>
	void Destroy();

	/// <summary>
	/// 当たり判定を行うか切り替える
	/// </summary>
	/// <param name="isCollision"></param>
	void SetCollisionState(bool isCollision);

	bool GetCollisionState();

	/// <summary>
	/// どのようなアイテムか返す
	/// </summary>
	/// <returns></returns>
	BuffType GetType() const { return _data.type; }

	/// <summary>
	/// 消滅しているか
	/// </summary>
	/// <returns></returns>
	bool IsAlive() { return _isAlive; }

private:

	// UpdateのStateパターン
	// _nowUpdateStateが変数であることを分かりやすくしている
	using UpdateFunc_t = void(ItemBase::*)();
	UpdateFunc_t _nowUpdateState;

	/// <summary>
	/// 生成中
	/// 1 -> 0
	/// </summary>
	void UpdateSpawning();
	/// <summary>
	/// 待機中
	/// プレイヤーが触れるか通知があるまで
	/// </summary>
	void UpdateIdle();
	/// <summary>
	/// 消滅中
	/// 0 -> 1
	/// </summary>
	void UpdateDestroying();

	void SetMatrix(Position3 pos, float rotSpeed);

	virtual void PlayGetSE() abstract;

protected:
	// モデルハンドル
	int _modelHandle;

	Vector3 _transOffset;
	Vector3 _scale;
	Vector3 _rotAngle;
	Vector3 _modelOffset;	// モデルの位置補正 当たり判定には関わらない
	Position3 _spawnPos;	// 生成位置(Yは生成後の位置)
	float _depthY;			// 生成時の深さ
	float _modelRotSpeed;	// 回転速度

	std::weak_ptr<PlayerBuffManager> _playerBuffManager;
	BuffData _data;
	bool _isAlive;

	int _animFrame;
	int _totalAnimFrame;
};

