#pragma once

#include "../SceneBase.h"

class Option : public SceneBase 
{
private:
public:
	Option();
	~Option();

	// ����������
	virtual void Initialize() override;
	int OptionImage;


	// �X�V����
	// �����F1�t���[��������̎���
	// �߂�l�F���̃V�[���^�C�v
	virtual eSceneType Update(float delta_second) override;

	// �`�揈��
	virtual void Draw()  override;

	// �I��������
	virtual void Finalize() override;

	// ���݂̃V�[���^�C�v�i�I�[�o�[���C�h�K�{�j
	virtual eSceneType GetNowSceneType() const override;
};