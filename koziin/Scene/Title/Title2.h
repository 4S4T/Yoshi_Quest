#pragma once

#include "../SceneBase.h"

class TitleScene2 : public SceneBase {
private:
public:
	TitleScene2();
	~TitleScene2();

	// ����������
	virtual void Initialize() override;
	int TitleImage;
	int Title_name;
	int select;
	int menu_cursor;


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