#pragma once
#include "PlayerStateBase.h"

/// <summary>
/// ��~���
/// </summary>
class IdleState : public PlayerStateBase {
private:
	// ���͏��
	class InputControl* input;

public:
	/// <summary>
	/// �R���X�g���N�^
	/// </summary>
	/// <param name="p">�v���C���[���</param>
	IdleState(class Player* p);

	// �f�X�g���N�^
	virtual ~IdleState();

public:
	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void Update(float delta_second) override;
	virtual void Draw() const override;

	// ���݂̃X�e�[�g�����擾����
	virtual ePlayerState GetState() const override;
};