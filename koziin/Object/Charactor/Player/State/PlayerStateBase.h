#pragma once
#include "Enum/PlayerState.h"

/// <summary>
/// �v���C���[��ԊǗ��p�̃x�[�X�N���X
/// </summary>
class PlayerStateBase {
protected:
	// �v���C���[���
	class Player* player;
	int image;

public:
	/// <summary>
	/// �R���X�g���N�^
	/// </summary>
	/// <param name="p">�v���C���[���</param>
	PlayerStateBase(class Player* p) : player(p), image(0) {
	}

	// �f�X�g���N�^
	virtual ~PlayerStateBase() = default;

public:
	virtual void Initialize() = 0;
	virtual void Finalize() = 0;
	virtual void Update(float delta_second) = 0;
	virtual void Draw() const = 0;


	// ��Ԃ̎擾
	virtual ePlayerState GetState() const = 0;
};