#pragma once

class Ground final {
public:
	Ground();
	~Ground();

	/// <summary>
	/// 描画全般
	/// </summary>
	void Draw();

private:

	int _modelHandle;
	int _textureHandle;
};