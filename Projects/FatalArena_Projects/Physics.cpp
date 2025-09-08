#include "Physics.h"
#include "Collider.h"
#include "ColliderData.h"
#include "ColliderDataSphere.h"
#include "ColliderDataCapsule.h"
#include "ColliderDataInvertedCylinder.h"
#include "Rigidbody.h"
#include "DebugDraw.h"
#include "Collision.h"

#include <cassert>
#include <vector>

void Physics::Entry(std::shared_ptr<Collider> collider)
{
	// (見つからなかった場合はend)
	auto it = (
		std::find(
			_colliders.begin(),
			_colliders.end(),
			collider));
	// 見つからなければ(登録されていなければ)
	if (it == _colliders.end())
	{
		_colliders.emplace_back(collider);	// 登録
	}
	// 既に登録されていたらassert
	else
	{
		assert(false && "指定のcolliderは登録済");
	}
}

void Physics::Release(std::shared_ptr<Collider> collider)
{
	// 登録解除(eraseif 要C++20)
	auto count = std::erase_if(
		_colliders,
		[collider](std::shared_ptr<Collider> target) { return target == collider; });
	// 登録されてなかったらassert
	if (count <= 0)
	{
		assert(false && "指定のcolliderは未登録");
	}
}

void Physics::Update()
{
	// 移動
	for (auto& collider : _colliders) {
		// 位置に移動量を足す
		Position3 pos = collider->rigidbody->GetPos();
		Vector3 vel = collider->rigidbody->GetVel();

		// 減速量を掛ける
		vel.x *= PhysicsData::decelerationRate * 0.5f;
		vel.z *= PhysicsData::decelerationRate * 0.5f;

		// 重力を利用するなら重力を与える
		if (collider->rigidbody->UseGravity()) {
			vel += PhysicsData::Gravity;

			// 最大重力加速度より小さかったら補正
			// (重力はマイナスのため)
			if (vel.y < PhysicsData::MaxGravityAccel.y) {
				vel.y = PhysicsData::MaxGravityAccel.y;
			}
		}

		// 移動量切り捨て処理
		Vector3 velXZ = vel;
		velXZ.y = 0.0f;
		// 移動していないとみなされる閾値よりも小さければ
		if (vel.Magnitude() < PhysicsData::sleepThreshold) {
			vel = {};
		}
		// XZのみを見て閾値よりも小さければ
		else if (velXZ.Magnitude() < PhysicsData::sleepThreshold) {
			vel.x = vel.z = 0.0f;
		}

		// もともとの情報、予定情報をデバッグ表示
#ifdef _DEBUG
		int color = 0xff00ff;
		// 当たらない場合は色を変える
		if (!(collider->colliderData->isCollision)) {
			color = 0x101010;
		}
		// 球
		if (collider->colliderData->GetKind() == PhysicsData::ColliderKind::Sphere)
		{
			auto sphereData = std::static_pointer_cast<ColliderDataSphere>(collider->colliderData);
			float radius = sphereData->_radius;
			DebugDraw::GetInstance().DrawSphere(pos, radius, color);
		}
		// カプセル
		if (collider->colliderData->GetKind() == PhysicsData::ColliderKind::Capsule)
		{
			auto capsuleData = std::static_pointer_cast<ColliderDataCapsule>(collider->colliderData);
			Position3 pos = collider->rigidbody->GetPos();
			Position3 start = capsuleData->GetStartPos(pos);
			Position3 end = capsuleData->GetEndPos(pos);
			float radius = capsuleData->GetRad();
			DebugDraw::GetInstance().DrawSphere(start, radius, color);
			DebugDraw::GetInstance().DrawSphere(end, radius, color);
			DebugDraw::GetInstance().DrawCapsule(start, end, radius, color);
		}
#endif

		// 予定位置、移動量設定
		Position3 nextPos = pos + vel;
		collider->rigidbody->SetVel(vel);
		collider->nextPos = nextPos;
	}

	// 当たり判定チェック（nextPos指定）
	std::list<OnCollideInfo> onCollideInfo = CheckCollide();

	// 位置確定
	FixPosition();

	// 当たり通知
	for (auto& info : onCollideInfo)
	{
		info.owner->OnCollide(info.colider);
	}
}

std::list<Physics::OnCollideInfo> Physics::CheckCollide() const
{
	std::list<OnCollideInfo> onCollideInfo;
	// 衝突通知、ポジション補正
	bool doCheck = true;
	int	checkCount = 0;	// チェック回数
	while (doCheck) {
		doCheck = false;
		++checkCount;

		// 2重ループで全オブジェクト当たり判定
		// 重いので近いオブジェクト同士のみ当たり判定するなど工夫がいる
		for (auto& objA : _colliders) {
			for (auto& objB : _colliders) {
				if (objA != objB) {
					// ぶつかっていれば
					if (IsCollide(objA, objB)) {
						auto priorityA = objA->GetPriority();
						auto priorityB = objB->GetPriority();

						std::shared_ptr<Collider> primary = objA;
						std::shared_ptr<Collider> secondary = objB;

						// どちらもトリガーでなければ次目標位置修正
						// (どちらかがトリガーなら補正処理を飛ばす)
						bool isTriggerAorB = objA->colliderData->IsTrigger() || objB->colliderData->IsTrigger();
						if (!isTriggerAorB) {
							// 移動優先度を数字に直したときに高い方を移動
							if (priorityA > priorityB) {
								primary = objB;
								secondary = objA;
							}
							// 位置補正を行う
							// priorityが同じだった場合は両方押し戻す
							FixNextPosition(primary, secondary, (priorityA == priorityB));
						}

						// 衝突通知情報の更新
						bool hasPrimaryInfo = false;
						bool hasSecondaryInfo = false;
						for (const auto& item : onCollideInfo) {
							// 既に通知リストに含まれていたら呼ばない
							if (item.owner == primary) {
								hasPrimaryInfo = true;
							}
							if (item.owner == secondary) {
								hasSecondaryInfo = true;
							}
						}
						if (!hasPrimaryInfo) {
							// MEMO:(実体作って入れるよりこっちの方が速そう)
							onCollideInfo.push_back({ primary, secondary });
						}
						if (!hasSecondaryInfo) {
							onCollideInfo.push_back({ secondary, primary });
						}

						// 一度でもヒット+補正したら衝突判定と補正やりなおし
						if (!isTriggerAorB) {	// 片方がトリガーならヒットとりなおさない
							doCheck = true;
						}
						break;
					}
				}
			}
			if (doCheck) {
				break;
			}
		}

		// 無限ループ避け
		if (checkCount > PhysicsData::kCheckCollideMaxCount && doCheck) {
#if _DEBUG
			//printfDx("当たり判定チェック回数が最大数(%d)を超えた\n",
			//	PhysicsData::kCheckCollideMaxCount);
#endif
			break;
		}
	}
	return onCollideInfo;
}

bool Physics::IsCollide(const std::shared_ptr<Collider> objA, const std::shared_ptr<Collider> objB) const
{
	bool isHit = false;

	// Colliderの種類によって、当たり判定を分ける
	auto aKind = objA->colliderData->GetKind();
	auto bKind = objB->colliderData->GetKind();

	auto aTag = objA->GetTag();
	auto bTag = objB->GetTag();

	// どちらかのオブジェクトが相手のタグを無視する設定になっていたらreturn
	if (objA->colliderData->IsThroughTarget(bTag) ||
		objB->colliderData->IsThroughTarget(aTag)) return false;

	if (!objA->colliderData->isCollision ||
		!objB->colliderData->isCollision) return false;

	// 球同士
	if (aKind == PhysicsData::ColliderKind::Sphere && bKind == PhysicsData::ColliderKind::Sphere)
	{
		auto atob = objB->nextPos - objA->nextPos;
		auto atobLength = atob.Magnitude();

		// お互いの距離が、それぞれの半径を足したものより小さければ当たる
		auto objAColliderData = std::static_pointer_cast<ColliderDataSphere>(objA->colliderData);
		auto objBColliderData = std::static_pointer_cast<ColliderDataSphere>(objB->colliderData);
		isHit = (atobLength < objAColliderData->_radius + objBColliderData->_radius);
	}
	// カプセル同士
	else if (aKind == PhysicsData::ColliderKind::Capsule && bKind == PhysicsData::ColliderKind::Capsule)
	{
		auto capsuleA = std::static_pointer_cast<ColliderDataCapsule>(objA->colliderData);
		auto capsuleB = std::static_pointer_cast<ColliderDataCapsule>(objB->colliderData);

		// カプセルAの線分と半径
		Vector3 startA = capsuleA->GetStartPos(objA->nextPos);
		Vector3 endA = capsuleA->GetEndPos(objA->nextPos);
		float radiusA = capsuleA->_radius;

		// カプセルBの線分と半径
		Vector3 startB = capsuleB->GetStartPos(objB->nextPos);
		Vector3 endB = capsuleB->GetEndPos(objB->nextPos);
		float radiusB = capsuleB->_radius;

		// 2つの線分の最近接点を求める
		Vector3 pA, pB;
		ClosestPointSegments(startA, endA, startB, endB, pA, pB);

		// 最近接点間の距離の2乗を計算
		float distSq = (pA - pB).SqrMagnitude();
		float radSum = radiusA + radiusB;

		// 最近接点間の距離が、半径の合計より小さいかどうかで衝突を判定
		isHit = distSq < (radSum * radSum);
	}
	// 反転円柱同士
	else if (aKind == PhysicsData::ColliderKind::InvertedCylinder && 
		bKind == PhysicsData::ColliderKind::InvertedCylinder)
	{
		// 未実装
	}
	// 球とカプセル
	else if ((aKind == PhysicsData::ColliderKind::Sphere && bKind == PhysicsData::ColliderKind::Capsule) ||
		(aKind == PhysicsData::ColliderKind::Capsule && bKind == PhysicsData::ColliderKind::Sphere))
	{
		// 球とカプセルを判定する
		std::shared_ptr<Collider> sphereObj;
		std::shared_ptr<Collider> capsuleObj;
		// objAが球であるかをチェック
		if (aKind == PhysicsData::ColliderKind::Sphere) {
			sphereObj = objA;
			capsuleObj = objB;
		}
		// でなければobjAはカプセル、objBが球
		else {
			sphereObj = objB;
			capsuleObj = objA;
		}

		// それぞれのコライダー情報を取得
		auto sphereData = std::static_pointer_cast<ColliderDataSphere>(sphereObj->colliderData);
		auto capsuleData = std::static_pointer_cast<ColliderDataCapsule>(capsuleObj->colliderData);

		// 球の情報を取得
		Vector3 sphereCenter = sphereObj->nextPos;
		float sphereRadius = sphereData->_radius;

		// カプセルの情報を取得
		Vector3 capsuleStart = capsuleData->GetStartPos(capsuleObj->nextPos);
		Vector3 capsuleEnd = capsuleData->GetEndPos(capsuleObj->nextPos);
		float capsuleRadius = capsuleData->_radius;

		// 点と線分の最近接点を求める
		Vector3 closestPointOnCapsuleAxis =
			ClosestPointPointAndSegment(
				sphereCenter,
				capsuleStart, capsuleEnd);

		// 最近接点間の距離の2乗を計算
		float distSq = (sphereCenter - closestPointOnCapsuleAxis).SqrMagnitude();
		// 半径の合計を計算
		float radSum = sphereRadius + capsuleRadius;

		// 距離が半径の合計より小さいか判定
		isHit = distSq < (radSum * radSum);
	}
	// 球と反転円柱
	else if ((aKind == PhysicsData::ColliderKind::Sphere && bKind == PhysicsData::ColliderKind::InvertedCylinder) ||
		(aKind == PhysicsData::ColliderKind::InvertedCylinder && bKind == PhysicsData::ColliderKind::Sphere))
	{
		// 未実装
	}
	// カプセルと反転円柱
	else if ((aKind == PhysicsData::ColliderKind::Capsule && bKind == PhysicsData::ColliderKind::InvertedCylinder) ||
		(aKind == PhysicsData::ColliderKind::InvertedCylinder && bKind == PhysicsData::ColliderKind::Capsule))
	{
		// 球とカプセルを判定する
		std::shared_ptr<Collider> capsuleObj;
		std::shared_ptr<Collider> cylinderObj;
		// objAがカプセルであるかをチェック
		if (aKind == PhysicsData::ColliderKind::Sphere) {
			cylinderObj = objA;
			capsuleObj = objB;
		}
		// でなければobjAは反転円柱、objBがカプセル
		else {
			cylinderObj = objB;
			capsuleObj = objA;
		}

		// それぞれのコライダー情報を取得
		auto capsuleData = std::static_pointer_cast<ColliderDataCapsule>(capsuleObj->colliderData);
		auto cylinderData = std::static_pointer_cast<ColliderDataInvertedCylinder>(cylinderObj->colliderData);

		// カプセル
		float capsuleRad = capsuleData->GetRad();
		Position3 capsuleStart = capsuleData->GetStartPos(capsuleObj->nextPos);
		Position3 capsuleEnd = capsuleData->GetEndPos(capsuleObj->nextPos);

		// 反転円柱
		float cylinderInnerRad = cylinderData->GetInnerRadius();
		float cylinderOuterRad = cylinderData->GetOuterRadius();
		Vector3 cylinderHeightVec = cylinderData->_startToEnd;
		Position3 cylinderStart = cylinderObj->nextPos;
		Position3 cylinderEnd = cylinderObj->nextPos + cylinderHeightVec;

		// 最近接点の計算
		// カプセルの中心線と、円柱の中心軸の最近接点を求める
		Position3 closestPointOnCapsule, closestPointOnCylinderAxis;
		ClosestPointSegments(
			capsuleStart, capsuleEnd,
			cylinderStart, cylinderEnd,
			closestPointOnCapsule, closestPointOnCylinderAxis);

		// 衝突判定
		// 最近接点間のXZ平面上でのベクトルと距離を計算
		Vector3 vecBetweenClosestPoints = closestPointOnCapsule - closestPointOnCylinderAxis;
		Vector2 vecXZ(vecBetweenClosestPoints.x, vecBetweenClosestPoints.z);
		float distXZ = vecXZ.Magnitude();

		bool isColideInner = distXZ < cylinderInnerRad - capsuleRad;
		bool isColideOuter = distXZ > cylinderOuterRad + capsuleRad;

		// 内側は当たっていないが外側は当たっている場合は当たっている
		// (上面/下面のフチとの判定は未実装)
		isHit = (!isColideInner && isColideOuter);
	}

	return isHit;
}

void Physics::FixNextPosition(std::shared_ptr<Collider> primary, std::shared_ptr<Collider> secondary, bool isMutualPushback) const
{
	// collidableの種類によって、当たり判定を分ける
	auto aKind = primary->colliderData->GetKind();
	auto bKind = secondary->colliderData->GetKind();

	// 球同士
	if (aKind == PhysicsData::ColliderKind::Sphere && bKind == PhysicsData::ColliderKind::Sphere)
	{
		// 当たり判定データ取得
		auto priColliderData = std::static_pointer_cast<ColliderDataSphere>(primary->colliderData);
		auto secColliderData = std::static_pointer_cast<ColliderDataSphere>(secondary->colliderData);

		// 押し戻し方向の決定
		// secondaryからprimaryへ向かうベクトルを計算し、正規化する
		Vector3 pushBackVec = primary->nextPos - secondary->nextPos;
		// 距離がゼロに近い場合は、押し戻し方向が不定になるため処理をスキップ
		if (pushBackVec.SqrMagnitude() < PhysicsData::kZeroTolerance) {
			return;
		}
		pushBackVec.Normalized();

		// 押し戻し距離(貫通深度)の計算
		// 現在の中心間の距離を計算
		float currentDist = (primary->nextPos - secondary->nextPos).Magnitude();
		// 2つの球の半径の合計
		float radiusSum = priColliderData->_radius + secColliderData->_radius;
		// 貫通深度にオフセットを加えた、最終的な押し戻し距離を計算
		float pushBackDist = (radiusSum - currentDist) + PhysicsData::kFixPositionOffset;

		// 位置の修正
		// 計算した方向と距離から、押し戻しベクトルを生成
		Vector3 fixVec = pushBackVec * pushBackDist;

		// 優先度が同じでお互いに押し戻す場合
		if (isMutualPushback) {
			// 押し戻し量を半分ずつに分ける
			Vector3 halfFixVec = fixVec * 0.5f;
			primary->nextPos += halfFixVec;
			secondary->nextPos -= halfFixVec;
		}
		// secondaryのみを押し戻す場合
		else {
			// secondaryを、primaryから離れる方向(pushBackVecの逆方向)に押し戻す
			secondary->nextPos -= fixVec;
		}
	}
	// カプセル同士
	else if (aKind == PhysicsData::ColliderKind::Capsule && bKind == PhysicsData::ColliderKind::Capsule)
	{
		// 当たり判定データ取得
		auto priCapsuleData = std::static_pointer_cast<ColliderDataCapsule>(primary->colliderData);
		auto secCapsuleData = std::static_pointer_cast<ColliderDataCapsule>(secondary->colliderData);
		// primaryカプセルの情報を取得
		Position3 priStart = priCapsuleData->GetStartPos(primary->nextPos);
		Position3 priEnd = priCapsuleData->GetEndPos(primary->nextPos);
		float priRadius = priCapsuleData->_radius;

		// secondaryカプセルの情報を取得
		Position3 secStart = secCapsuleData->GetStartPos(secondary->nextPos);
		Position3 secEnd = secCapsuleData->GetEndPos(secondary->nextPos);
		float secRadius = secCapsuleData->_radius;

		// 最近傍点の計算
		// 2つのカプセルの中心線上で最も近い点(pPri, pSec)を計算
		Position3 pPri, pSec;
		ClosestPointSegments(priStart, priEnd, secStart, secEnd, pPri, pSec);

		// 押し戻し方向の決定
		// 最近傍点間のベクトルを計算し、押し戻し方向を決定
		Vector3 pushBackVec = pSec - pPri;
		// 距離がゼロに近い場合は、カプセルの中心位置から方向を仮決めする（めり込みきっている場合など）
		if (pushBackVec.SqrMagnitude() < PhysicsData::kZeroTolerance) {
			pushBackVec = secondary->nextPos - primary->nextPos;
		}
		pushBackVec.Normalized();

		// 押し戻し距離(貫通深度)の計算
		// 最近傍点間の現在の距離を計算
		float currentDist = (pSec - pPri).Magnitude();
		// 2つのカプセルの半径の合計
		float radiusSum = priRadius + secRadius;
		// 貫通深度にオフセットを加えた、最終的な押し戻し距離を計算
		float pushBackDist = (radiusSum - currentDist) + PhysicsData::kFixPositionOffset;

		// 位置の修正
		// 計算した方向と距離を使って、カプセルの位置を修正
		Vector3 fixVec = pushBackVec * pushBackDist;

		// 優先度が同じでお互いに押し戻す場合
		if (isMutualPushback) {
			// 押し戻し量を半分ずつに分ける
			Vector3 halfFixVec = fixVec * 0.5f;
			primary->nextPos -= halfFixVec;
			secondary->nextPos += halfFixVec;
		}
		// secondaryのみを押し戻す場合
		else {
			secondary->nextPos += fixVec;
		}
	}
	// 反転円柱同士
	else if (aKind == PhysicsData::ColliderKind::InvertedCylinder &&
		bKind == PhysicsData::ColliderKind::InvertedCylinder)
	{
		// 未実装
	}
	// 球とカプセル
	else if ((aKind == PhysicsData::ColliderKind::Sphere && bKind == PhysicsData::ColliderKind::Capsule) ||
		(aKind == PhysicsData::ColliderKind::Capsule && bKind == PhysicsData::ColliderKind::Sphere))
	{
		// 当たり判定データ取得
		std::shared_ptr<Collider> sphereObj;
		std::shared_ptr<Collider> capsuleObj;
		// primaryとsecondaryがそれぞれ球かカプセルかを判別
		if (primary->colliderData->GetKind() == PhysicsData::ColliderKind::Sphere) {
			sphereObj = primary;
			capsuleObj = secondary;
		}
		else {
			sphereObj = secondary;
			capsuleObj = primary;
		}

		// それぞれのColliderからデータを取得
		auto sphereData = std::static_pointer_cast<ColliderDataSphere>(sphereObj->colliderData);
		auto capsuleData = std::static_pointer_cast<ColliderDataCapsule>(capsuleObj->colliderData);

		// 球の情報を取得
		Vector3 sphereCenter = sphereObj->nextPos;
		float sphereRadius = sphereData->_radius;

		// カプセルの情報を取得
		Vector3 capsuleStart = capsuleData->GetStartPos(capsuleObj->nextPos);
		Vector3 capsuleEnd = capsuleData->GetEndPos(capsuleObj->nextPos);
		float capsuleRadius = capsuleData->_radius;

		// 最近傍点の計算
		// 球の中心とカプセルの中心線との最近傍点を計算
		Vector3 closestPointOnCapsuleAxis = ClosestPointPointAndSegment(sphereCenter, capsuleStart, capsuleEnd);

		// 押し戻し方向の決定
		// カプセルの最近傍点から球の中心へ向かうベクトルを、押し戻し方向とする
		Vector3 pushBackVec = sphereCenter - closestPointOnCapsuleAxis;
		// 距離がゼロに近い場合は、オブジェクトの中心位置から方向を仮決めする
		if (pushBackVec.SqrMagnitude() < PhysicsData::kZeroTolerance) {
			pushBackVec = sphereObj->nextPos - capsuleObj->nextPos;
		}
		pushBackVec.Normalized();

		// 押し戻し距離(貫通深度)の計算
		// 最近傍点間の現在の距離を計算
		float currentDist = (sphereCenter - closestPointOnCapsuleAxis).Magnitude();
		// 2つのオブジェクトの半径の合計
		float radiusSum = sphereRadius + capsuleRadius;
		// 貫通深度にオフセットを加えた、最終的な押し戻し距離を計算
		float pushBackDist = (radiusSum - currentDist) + PhysicsData::kFixPositionOffset;

		// 位置の修正
		// 計算した方向と距離を使って、オブジェクトの位置を修正
		Vector3 fixVec = pushBackVec * pushBackDist;

		// 優先度が同じでお互いに押し戻す場合
		if (isMutualPushback) {
			// 押し戻し量を半分ずつに分け、それぞれを押し戻す
			Vector3 halfFixVec = fixVec * 0.5f;
			sphereObj->nextPos += halfFixVec;	// 球を押し戻し
			capsuleObj->nextPos -= halfFixVec;	// カプセルを押し戻し
		}
		// 優先度に従って片方のみを押し戻す場合
		else {
			// 優先度の低い方(secondary)を押し戻す
			// (fixVecはカプセルから球へのベクトルのため、足し引きを使い分ける)
			if (secondary == sphereObj) {
				secondary->nextPos += fixVec;	// secondary(球)を押し戻す
			}
			else {	// secondary == capsuleObj
				secondary->nextPos -= fixVec;	// secondary(カプセル)を押し戻す
			}
		}
	}
	// 球と反転円柱
	else if ((aKind == PhysicsData::ColliderKind::Sphere && bKind == PhysicsData::ColliderKind::InvertedCylinder) ||
		(aKind == PhysicsData::ColliderKind::InvertedCylinder && bKind == PhysicsData::ColliderKind::Sphere))
	{
			// 未実装
	}
	// カプセルと反転円柱
	else if ((aKind == PhysicsData::ColliderKind::Capsule && bKind == PhysicsData::ColliderKind::InvertedCylinder) ||
		(aKind == PhysicsData::ColliderKind::InvertedCylinder && bKind == PhysicsData::ColliderKind::Capsule))
	{
		// 押し戻し量
		Vector3 fixVec;
		// 球とカプセルを判定する
		std::shared_ptr<Collider> capsuleObj;
		std::shared_ptr<Collider> cylinderObj;
		// primaryとsecondaryがそれぞれカプセルか反転円柱かを判別
		if (primary->colliderData->GetKind() == PhysicsData::ColliderKind::Capsule) {
			capsuleObj = primary;
			cylinderObj = secondary;
		}
		else {
			capsuleObj = secondary;
			cylinderObj = primary;
		}

		// それぞれのコライダー情報を取得
		auto capsuleData = std::static_pointer_cast<ColliderDataCapsule>(capsuleObj->colliderData);
		auto cylinderData = std::static_pointer_cast<ColliderDataInvertedCylinder>(cylinderObj->colliderData);

		// カプセル
		float capsuleRad = capsuleData->GetRad();
		Position3 capsuleStart = capsuleData->GetStartPos(capsuleObj->nextPos);
		Position3 capsuleEnd = capsuleData->GetEndPos(capsuleObj->nextPos);

		// 反転円柱
		float cylinderInnerRad = cylinderData->GetInnerRadius();
		float cylinderOuterRad = cylinderData->GetOuterRadius();
		Vector3 cylinderHeightVec = cylinderData->_startToEnd;
		Position3 cylinderStart = cylinderObj->nextPos - cylinderHeightVec * 0.5f;
		Position3 cylinderEnd = cylinderObj->nextPos + cylinderHeightVec * 0.5f;

		// 最近接点の計算
		// カプセルの中心線と、円柱の中心軸の最近接点を求める
		Position3 closestPointOnCapsule, closestPointOnCylinderAxis;
		ClosestPointSegments(
			capsuleStart, capsuleEnd,
			cylinderStart, cylinderEnd,
			closestPointOnCapsule, closestPointOnCylinderAxis);

		// 衝突判定
		// 最近接点間のXZ平面上でのベクトルと距離を計算
		Vector3 vecBetweenClosestPoints = closestPointOnCapsule - closestPointOnCylinderAxis;
		Vector2 vecXZ(vecBetweenClosestPoints.x, vecBetweenClosestPoints.z);
		float distXZ = vecXZ.Magnitude();

		bool isColideInner = distXZ < cylinderInnerRad - capsuleRad;
		bool isColideOuter = distXZ > cylinderOuterRad + capsuleRad;
		// 内側は当たっていないが外側は当たっている状態(当たっている)
		// 上記の状態でない場合(当たっていない場合)return
		// (上面/下面のフチとの判定は未実装)
		bool isHit = !isColideInner && isColideOuter;
		if (!isHit) return;

		// 衝突している場合
		// 貫通深度と押し戻し方向を計算
		float penetration = 0.0f;
		Vector3 pushDir = { vecBetweenClosestPoints.x, 0.0f, vecBetweenClosestPoints.z };
		if (pushDir.SqrMagnitude() < PhysicsData::kZeroTolerance) {
			// ほぼ真上/真下にいる場合、仮の方向を設定
			pushDir = { 1.0f, 0.0f, 0.0f };
		}
		pushDir.Normalized();

		// 内側と外側のどちらにめり込んでいるか判定
		float distToInner = std::abs(distXZ - cylinderInnerRad);
		float distToOuter = std::abs(distXZ - cylinderOuterRad);

		if (distToInner < distToOuter) {
			// 内側にめり込んでいる場合は外側へ押し出す
			penetration = (cylinderInnerRad - distXZ) + capsuleRad;
			fixVec = pushDir * penetration;
		}
		else {
			// 外側にめり込んでいる場合は内側へ押し出す
			penetration = (cylinderOuterRad - distXZ) + capsuleRad;
			fixVec = -pushDir * penetration;
		}

		// 補正量分追加で押し戻す
		fixVec *= 1.0f + PhysicsData::kFixPositionOffset;

		// 優先度が同じでお互いに押し戻す場合
		if (isMutualPushback) {
			Vector3 halfFixVec = fixVec * 0.5f;
			// 押し戻しベクトルは円柱→カプセルの方向で計算されている
			capsuleObj->nextPos += halfFixVec;
			cylinderObj->nextPos -= halfFixVec;
		}
		else {
			// 優先度の低い方(secondary)を押し戻す
			if (secondary == capsuleObj) {
				secondary->nextPos += fixVec;
			}
			else { // secondary == cylinderObj
				secondary->nextPos -= fixVec;
			}
		}

		return;
	}
}

void Physics::FixPosition()
{
	for (auto& collider : _colliders) {
		Vector3 toFixedPos;
		if (true) {
			// 床判定を無理やり作る
			if (collider->nextPos.y <= 0.0f) {
				collider->nextPos.y = 0.0f;
			}
		}
		else {

		}
		// Posを更新するので、velocityもそこに移動するvelocityに修正
		toFixedPos = collider->nextPos - collider->rigidbody->GetPos();

		collider->rigidbody->SetVel(toFixedPos);

		// 位置確定
		collider->rigidbody->SetPos(collider->nextPos);
	}
}
