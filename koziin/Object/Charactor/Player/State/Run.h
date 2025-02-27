#pragma once
#include "PlayerStateBase.h"
#include "../../../../Utility/Vector2D.h"
#include<vector>

/// <summary>
/// �W�����v���
/// </summary>
class RunState : public PlayerStateBase {
private:
	std::vector<int> player_animation; // �ړ��̃A�j���[�V�����摜
	int animation[2];				   // �A�j���[�V�����摜
	// �O�t���[���̍��W�擾
	Vector2D old_location;
	float animation_time; // �A�j���[�V��������
	int animation_count;  // �A�j���[�V�����Y��

	// �ړ��A�j���[�V�����̏���
	const int animation_num[1] = 
	{
		0
	};
	public:
	/// <summary>
	/// �R���X�g���N�^
	/// </summary>
	/// <param name="p">�v���C���[���</param>
	RunState(class Player* p);

	// �f�X�g���N�^
	virtual ~RunState();

public:
	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void Update(float delta_second) override;
	virtual void Draw() const override;
	void AnimationControl(float delta_second);

	// ���݂̃X�e�[�g�����擾����
	virtual ePlayerState GetState() const override;
};