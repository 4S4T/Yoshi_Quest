#pragma once
#include "SceneBase.h"
#include "../Singleton.h"
#include"../Object/Charactor/Player/Player.h"

class SceneManager : public Singleton<class Type> {
private:
	SceneBase* current_scene; // ���݂̃V�[�����̃|�C���^
	/*static SceneManager* instance;*/
private:
	Player* player;


public:
	SceneManager();
	~SceneManager();

	static void DeleteInstance();

	// ����������
	void Initialize();

	// �X�V����
	bool Update(float delta_second);

	// �I��������
	void Finalize();

	// �V�[���ؑ֏���
	void ChangeScene(eSceneType);

public:
	// �`�揈��
	void Draw() const;


	// �V�[���ؑ֏���
	SceneBase* CreateScene(eSceneType type);
};