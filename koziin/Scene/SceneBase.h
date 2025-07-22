#pragma once
#include "../Object/GameObjectManager.h"

enum eSceneType {
	eTitle,
	eTitle2,
	eMap,
	eResult,
	eOption,
	eBattle,
	eEnd,
};

class SceneBase {
public:
	SceneBase() {}
	~SceneBase() {}

	// ����������
	virtual void Initialize() {}

	// �X�V����
	// �����F1�t���[��������̎���
	// �߂�l�F���̃V�[���^�C�v
	virtual eSceneType Update(float aaaa) {
		return GetNowSceneType();
	}

	// �`�揈��
	virtual void Draw() 
	{
		GameManager* obj = Singleton<GameManager>::GetInstance();
		obj->Draw();
	}

	// �I��������
	virtual void Finalize() 
	{

	}

	// ���݂̃V�[���^�C�v�i�I�[�o�[���C�h�K�{�j
	virtual eSceneType GetNowSceneType() const = 0;
};