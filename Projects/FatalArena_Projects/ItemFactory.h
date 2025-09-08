#pragma once
#include "Vector3.h"
#include "PlayerBuffManager.h"

#include <memory>
#include <unordered_map>

class ItemBase;
class Physics;

/// <summary>
/// �w�肳�ꂽ�A�C�e���̐����⃂�f���n���h�����Ǘ����Ă���
/// </summary>
class ItemFactory final {
public:
	/// <summary>
	/// �K�v�ȃ��f�������ׂēǂݍ���
	/// </summary>
	static void LoadResources();

	/// <summary>
	/// �ǂݍ��񂾃��f�������ׂĉ������
	/// </summary>
	static void ReleaseResources();

	/// <summary>
	/// �G�̎�ނɉ����ăC���X�^���X�𐶐����A
	/// ��������physics�ւ̓o�^���s��
	/// </summary>
	static std::shared_ptr<ItemBase> CreateAndRegister(
		BuffType type,
		const Vector3& position,
		std::weak_ptr<PlayerBuffManager> manager,
		std::weak_ptr<Physics> physics
	);

	static float GetStrengthMulAmount();

private:
	// ���f���n���h�����Ǘ����邽�߂̃R���e�i
	// �L�[:�G�̎��, �l:���f���n���h��
	static std::unordered_map<BuffType, int> _modelHandles;
};

