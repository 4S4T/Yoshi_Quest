#pragma once
#include "SceneBase.h"
#include "../Singleton.h"

class SceneManager : public Singleton<class Type> {
private:
	SceneBase* current_scene; // ���݂̃V�[�����̃|�C���^
	/*static SceneManager* instance;*/

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