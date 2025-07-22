#include "SceneManager.h"
#include "../Utility/InputControl.h"
#include "DxLib.h"
#include"Title/Title.h"
#include"Title/Title2.h"
#include"Option/Option.h"
#include"Map/Map.h"
#include"Battle/Battle.h"

SceneManager::SceneManager() {
}

SceneManager::~SceneManager() {
	// ����Y��h�~
	Finalize();
}

void SceneManager::DeleteInstance() {
	static SceneManager* instance;

	if (instance != nullptr) {
		delete instance;
		instance = nullptr;
	}
}

/// <summary>
/// ����������
/// </summary>
void SceneManager::Initialize() {
}

bool SceneManager::Update(float delta_second) {
	// �V�[���̍X�V����
	eSceneType next_scene_type = current_scene->Update(delta_second);

	if (next_scene_type == eSceneType::eEnd)
	{
		return true;
	}

	// �V�[���̕`�揈��
	Draw();

	// �V�[���؂�ւ�
	if (next_scene_type != current_scene->GetNowSceneType()) {
		ChangeScene(next_scene_type);
	}

	return false;
}

/// <summary>
/// �I��������
/// </summary>
void SceneManager::Finalize() {
	// �V�[����񂪐�������Ă���΁A�폜����
	if (current_scene != nullptr) {
		current_scene->Finalize();
		delete current_scene;
		current_scene = nullptr;
	}

	// ���͋@�\�̃C���X�^���X���폜
	InputControl::DeleteInstance();

	// DX���C�u�����̏I������
	DxLib_End();
}

/// <summary>
/// �`�揈��
/// </summary>
void SceneManager::Draw() const {
	// �V�[���̕`�揈��
	current_scene->Draw();
}

/// <summary>
/// �V�[���ؑ֏���
/// </summary>
/// <param name="type">�V�[�����</param>
void SceneManager::ChangeScene(eSceneType type) {
	SceneBase* new_scene = CreateScene(type);
	if (new_scene == nullptr) {
		throw("�V�[���������ł��܂���ł���\n");
	}

	if (type == eSceneType::eBattle) {
		Map* mapScene = dynamic_cast<Map*>(current_scene);
		BattleScene* battleScene = dynamic_cast<BattleScene*>(new_scene);

		if (!mapScene) {
			printf("Error: current_scene is not a Map instance!\n");
			return;
		}
		if (!battleScene) {
			printf("Error: new_scene is not a BattleScene instance!\n");
			return;
		}

		if (!mapScene->player) {
			printf("Error: mapScene->player is nullptr! Initializing new player.\n");
			mapScene->player = new Player(); // ���̏������i�������Ǘ��ɒ��Ӂj
		}

		battleScene->SetPlayerPosition(mapScene->player->GetLocation());
		battleScene->SetPlayer(mapScene->player);
	}


	if (current_scene != nullptr) {
		current_scene->Finalize();
		current_scene = new_scene;
	}

	new_scene->Initialize();
	current_scene = new_scene;
}



/// <summary>
/// �V�[����������
/// </summary>
/// <param name="type">�V�[�����</param>
/// <returns>�V�[���N���X�̃|�C���g</returns>
SceneBase* SceneManager::CreateScene(eSceneType type) {

	// �V�[�����ɂ���āA��������V�[����؂�ւ���
	switch (type) {
	case eTitle:
		return dynamic_cast<SceneBase*>(new TitleScene());
	case eTitle2:
		return dynamic_cast<SceneBase*>(new TitleScene2());
	case eMap:
		return dynamic_cast<SceneBase*>(new Map());
	case eOption:
		return dynamic_cast<SceneBase*>(new Option());
	case eBattle:
		return dynamic_cast<SceneBase*>(new BattleScene());

	default:
		return nullptr;
	}
}
