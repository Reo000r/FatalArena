#include "Collision.h"
#include "ProjectSettings.h"

#include <algorithm>

Position3 ClosestPointPointAndSegment(const Position3& point, const Position3& start, const Position3& end)
{
	// 線分の始点から終点へのベクトル
	Vector3 segmentVec = end - start;
	// 線分の始点から指定された点へのベクトル
    Vector3 pointToStart = point - start;
    
    // 線分の長さの2乗
    float segmentLengthSq = segmentVec.SqrMagnitude();
    
    // 線分の長さがほぼ0の場合、始点が最近接点
	if (segmentLengthSq < PhysicsData::kZeroTolerance) {
		return start;
	}
	
	// 点を線分ベクトルに射影したときの t を計算
	// t = (point - start)・(end - start) / |end - start| ^ 2
	float t = Dot(pointToStart, segmentVec) / segmentLengthSq;
	
	// t を0.0～1.0の範囲にクランプする
	t = std::clamp(t, 0.0f, 1.0f);
	
	// 最近接点を計算して返す
	return (start + segmentVec * t);
}

void ClosestPointSegments(const Position3& startA, const Position3& endA, const Position3& startB, const Position3& endB, Position3& closestPointA, Position3& closestPointB)
{
	// 2つの線分の最近接点を求める処理
	Vector3 segAVec = endA - startA;	// 線分Aの方向ベクトル
	Vector3 segBVec = endB - startB;	// 線分Bの方向ベクトル
	Vector3 offsetVec = startA - startB;

	float segALenSq = segAVec.SqrMagnitude();	// 線分Aの長さの2乗
	float segBLenSq = segBVec.SqrMagnitude();	// 線分Bの長さの2乗
	float segBDotOffset = Dot(segBVec, offsetVec);	// 線分Bの方向ベクトルと両始点間ベクトルの内積

	// 両方の線分が点とみなせる場合
	if (segALenSq <= PhysicsData::kZeroTolerance && segBLenSq <= PhysicsData::kZeroTolerance) {
		// 両方が点の場合
		closestPointA = startA;
		closestPointB = startB;
		return;		// 処理終了
	}
	// 線分Aのみが点の場合
	else if (segALenSq <= PhysicsData::kZeroTolerance) {
		closestPointA = startA;
		closestPointB = ClosestPointPointAndSegment(startA, startB, endB);
		return;		// 処理終了
	}
	// 線分Bが点である場合
	else if (segBLenSq <= PhysicsData::kZeroTolerance) {
		closestPointA = ClosestPointPointAndSegment(startB, startA, endA);
		closestPointB = startB;
		return;		// 処理終了
	}

	// 両方が線分として扱える場合
	// 線分Aの方向ベクトルと両始点間ベクトルの内積
	float segADotOffset = Dot(segAVec, offsetVec);
	// 線分Aと線分Bの方向ベクトルの内積
	float segADotSegB = Dot(segAVec, segBVec);
	// 線分上の最近接点を求めるための変数 (0.0～1.0)
	float paramA, paramB;

	// 最近接点のパラメータを計算
	CalculateClosestSegmentParameters(
		segALenSq, segBLenSq, segADotSegB, segADotOffset, segBDotOffset, 
		paramA, paramB);

	// 求めた値を用いて、線分上の最近接点を求める
	closestPointA = startA + segAVec * paramA;
	closestPointB = startB + segBVec * paramB;
}

void CalculateClosestSegmentParameters(
	float segALenSq, float segBLenSq, float segADotSegB,
	float segADotOffset, float segBDotOffset,
	float& paramA, float& paramB)
{
	// 最近接点を求める式の分母
	float denom = segALenSq * segBLenSq - segADotSegB * segADotSegB;

	// 二直線が平行でない場合
	if (std::abs(denom) > PhysicsData::kZeroTolerance) {
		// 直線上の最近接点を計算
		paramA = (segADotSegB * segBDotOffset - segADotOffset * segBLenSq) / denom;
		paramB = (segALenSq * segBDotOffset - segADotSegB * segADotOffset) / denom;
	}
	// 線分が平行な場合
	else {
		// 計算を簡略化するため、片方の始点を基準に計算する
		paramA = 0.0f;
		paramB = segBDotOffset / segBLenSq;
	}

	// 計算されたパラメータが両方とも 0-1 の範囲内にあるかチェック
	// 最近接点が両線分の内部にあるケース
	if (paramA >= 0.0f && paramA <= 1.0f && paramB >= 0.0f && paramB <= 1.0f) {
		// 範囲内なので何もしない
		return;
	}

	// それ以外の場合、最近接点は少なくとも片方の線分の端点にある
	// 最近接点が線分の外側にある場合、端点にクランプして再計算する

	// paramA を 0-1 にクランプ
	paramA = std::clamp(paramA, 0.0f, 1.0f);
	// クランプした paramA を元に paramB を再計算
	paramB = (segBDotOffset + segADotSegB * paramA) / segBLenSq;
	paramB = std::clamp(paramB, 0.0f, 1.0f);

	// さらに、クランプした paramB を元に paramA を再々計算
	// これにより、両方の範囲が互いに最適な状態に収束する
	paramA = (-segADotOffset + segADotSegB * paramB) / segALenSq;
	paramA = std::clamp(paramA, 0.0f, 1.0f);

	return;
}