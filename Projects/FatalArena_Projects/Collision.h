#pragma once
#include "Geometry.h"

// 当たり判定系の処理を行う関数をまとめたファイル



/// <summary>
/// 点と線分の最近接点を求める
/// </summary>
/// <param name="point">点</param>
/// <param name="start">線分の始点</param>
/// <param name="end">線分の終点</param>
/// <returns>線分上の最近接点</returns>
Position3 ClosestPointPointAndSegment(const Position3& point, 
	const Position3& start, const Position3& end);

/// <summary>
/// 2つの線分の最近接点を求める
/// </summary>
/// <param name="startA">線分A始点</param>
/// <param name="endA">線分A終点</param>
/// <param name="startB">線分B始点</param>
/// <param name="endB">線分B終点</param>
/// <param name="closestPointA">A上の最近接点</param>
/// <param name="closestPointB">B上の最近接点</param>
void ClosestPointSegments(
	const Position3& startA, const Position3& endA,
	const Position3& startB, const Position3& endB,
	Position3& closestPointA, Position3& closestPointB);

/// <summary>
/// 2つの線分の最近接点を求めるためのパラメータを計算する
/// (ClosestPointSegmentsの補助関数)
/// </summary>
/// <param name="lenSq1">線分1の長さの2乗</param>
/// <param name="lenSq2">線分2の長さの2乗</param>
/// <param name="dot12">線分ベクトル同士の内積</param>
/// <param name="dot1r">線分1とオフセットベクトルの内積</param>
/// <param name="dot2r">線分2とオフセットベクトルの内積</param>
/// <param name="param1">出力用のパラメータ1</param>
/// <param name="param2">出力用のパラメータ2</param>
void CalculateClosestSegmentParameters(
	float lenSq1, float lenSq2, float dot12,
	float dot1r, float dot2r,
	float& param1, float& param2);
