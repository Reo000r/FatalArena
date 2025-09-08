#include "Ground.h"
#include "Geometry.h"
#include <DxLib.h>
#include <vector>

namespace {
	constexpr int kMaxXZ = 1000;
	constexpr int kDivNum = 30;
	constexpr int kPerOneDist = (kMaxXZ * 2) / kDivNum;


	const float kFieldWidth = 3000.0f; // 四角形の幅(X軸方向)
	const float kFieldDepth = 3000.0f; // 四角形の奥行き(Z軸方向)
	const float kFieldY = 0.0f;        // 地面のY座標
	const unsigned int kFieldColor = GetColor(255, 255, 255); // 地面の色
	const Position3 kFieldCenter = VGet(0.0f, 0.0f, 700.0f);

}

Ground::Ground() :
	_modelHandle(-1),
	_textureHandle(-1)
{
	_modelHandle = MV1LoadModel(L"data/model/field/Arena.mv1");
	_textureHandle = LoadGraph(L"data/texture/Texture_Ground.jpg");

	MV1SetPosition(_modelHandle, Vector3(0, 0, 0));
	MV1SetScale(_modelHandle, Vector3(1, 1, 1) * 0.5f);
}

Ground::~Ground() 
{
	MV1DeleteModel(_modelHandle);
	DeleteGraph(_textureHandle);
}

void Ground::Draw() 
{
	MV1DrawModel(_modelHandle);

#if true

	// 四角形を構成する4つの頂点を定義
	// 左奥、右奥、左手前、右手前
	VERTEX3D vertices[4];
	float w = kFieldWidth / 2.0f;
	float d = kFieldDepth / 2.0f;

	// 頂点座標
	vertices[0].pos = VGet(kFieldCenter.x - w, kFieldCenter.y, kFieldCenter.z + d); // 左奥
	vertices[1].pos = VGet(kFieldCenter.x + w, kFieldCenter.y, kFieldCenter.z + d); // 右奥
	vertices[2].pos = VGet(kFieldCenter.x - w, kFieldCenter.y, kFieldCenter.z - d); // 左手前
	vertices[3].pos = VGet(kFieldCenter.x + w, kFieldCenter.y, kFieldCenter.z - d); // 右手前

	// 全頂点に共通の設定を適用
	for (int i = 0; i < 4; ++i) {
		vertices[i].dif = GetColorU8(
			kFieldColor >> 16,          // 赤(R)成分の取得: 16ビット右にずらす
			(kFieldColor >> 8) & 0xFF,  // 緑(G)成分の取得: 8ビット右にずらし、下位8ビットを取り出す
			kFieldColor & 0xFF,         // 青(B)成分の取得: 下位8ビットを取り出す
			255                         // アルファ(A)成分: 255で完全不透明
		);
		vertices[i].spc = GetColorU8(0, 0, 0, 0);
		vertices[i].norm = VGet(0.0f, 1.0f, 0.0f); // 法線はすべて上向き
		vertices[i].u = 0.0f;	// 初期化
		vertices[i].v = 0.0f;
	}

	// UV座標を設定
	vertices[0].u = 0.0f; vertices[0].v = 0.0f; // 左奥
	vertices[1].u = 1.0f; vertices[1].v = 0.0f; // 右奥
	vertices[2].u = 0.0f; vertices[2].v = 1.0f; // 左手前
	vertices[3].u = 1.0f; vertices[3].v = 1.0f; // 右手前

	// 1つ目の三角形 (左奥、右奥、左手前)
	VERTEX3D triangle1[3] = { vertices[0], vertices[1], vertices[2] };
	DrawPolygon3D(triangle1, 1, _textureHandle, true);

	// 2つ目の三角形 (右手前、左手前、右奥)
	VERTEX3D triangle2[3] = { vertices[3], vertices[2], vertices[1] };
	DrawPolygon3D(triangle2, 1, _textureHandle, true);

#else
	unsigned int color = 0xffffff;
	Vector3 pos1, pos2;
	pos1 = { 0,0,0 };
	pos2 = { 0,400,0 };
	DrawSphere3D(pos1, 10, 16, 0x888888, 0xff00ff, true);
	DrawLine3D(pos1, pos2, color);
	pos1 = { -kMaxXZ, 0,  kMaxXZ };
	pos2 = { -kMaxXZ, 0, -kMaxXZ };
	color = 0x00cc00;
	for (int i = 0; i < kDivNum + 1; ++i) {
		DrawLine3D(pos1, pos2, color);
		pos1.x += kPerOneDist;
		pos2.x = pos1.x;
	}
	pos1 = { kMaxXZ, 0, -kMaxXZ };
	pos2 = { -kMaxXZ, 0, -kMaxXZ };
	color = 0x0000cc;
	for (int i = 0; i < kDivNum + 1; ++i) {
		DrawLine3D(pos1, pos2, color);
		pos1.z += kPerOneDist;
		pos2.z = pos1.z;
	}
#endif
}
