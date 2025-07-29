#pragma once

#include "../SceneBase.h"

enum class SubMenuState {
	None,	// �����I������ĂȂ����j���[���
	Option, // �I�v�V�����\����
	Credit	// �N���W�b�g�\����
};

class Memu : public SceneBase {
private:
	int currentSelection;
	const char* menuItems[3] = { "�ݒ�", "�N���W�b�g", "�}�b�v�ɖ߂�" };
	const int menuItemCount = 3;
	SubMenuState subMenuState; // �T�u���j���[���

public:
	Memu();
	~Memu();

	// ����������
	virtual void Initialize() override;
	int OptionImage;


	// �X�V����
	// �����F1�t���[��������̎���
	// �߂�l�F���̃V�[���^�C�v
	virtual eSceneType Update(float delta_second) override;

	// �`�揈��
	virtual void Draw() override;

	// �I��������
	virtual void Finalize() override;

	// ���݂̃V�[���^�C�v�i�I�[�o�[���C�h�K�{�j
	virtual eSceneType GetNowSceneType() const override;
};