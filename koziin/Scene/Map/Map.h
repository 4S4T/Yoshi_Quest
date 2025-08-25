#pragma once

#include "../SceneBase.h"
#include "../../Utility/Vector2D.h"
#include "../../Object/Charactor/Player/Player.h"
#include "../../Object/NPC/NPC.h"
#include <memory>
#include <string>
#include <vector>
#include <random>
#include "../../Object/Item/Item.h"

class Map : public SceneBase {
private:
	// �}�b�v�J�ڃ|�C���g�̃��X�g
	std::vector<Vector2D> transitionPoints;

	// �Փ˔���}�b�v�itrue: �Փ˂���, false: �Փ˂Ȃ��j
	std::vector<std::vector<bool>> collisionMap;

	// �X�e�[�W2��p NCP �Q
	std::vector<std::shared_ptr<NCP>> ncps;


	// �X�e�[�W�f�[�^�t�@�C���̃��X�g
	std::vector<std::string> stageFiles = {
		"Resource/stage1.csv",
		"Resource/stage2.csv",
		"Resource/stage3.csv"
	};

	// ���݂̃X�e�[�W�C���f�b�N�X
	int currentStageIndex = 0;

	// �J�����̃I�t�Z�b�g�i�v���C���[�̈ʒu�ɉ����ăX�N���[���j
	Vector2D cameraOffset;
	const float SCREEN_WIDTH = 800.0f;
	const float SCREEN_HEIGHT = 600.0f;

private:
	int encounterStepCounter = 0;		   // �v���C���[�̈ړ��X�e�b�v��
	Vector2D lastPlayerPos;				   // �v���C���[�̍Ō�̈ʒu
	const int encounterStepThreshold = 10; // 10�����ƂɃG���J�E���g����

	bool isMenuVisible = false; // ���j���[�\�����t���O
	int menuSelection = 0;		// ���ݑI�𒆂̃��j���[����
	const char* menuItems[4] = { "�ǂ���", "������", "�悳", "���ǂ�" };
	const int menuItemCount = 4;

	bool isSubMenuVisible = false; // �T�u���j���[�\�����t���O
	std::string subMenuText = "";  // �\��������e

	std::vector<std::shared_ptr<Item>> items;	 // �}�b�v��ɔz�u�����A�C�e��
	//std::vector<std::string> collectedItemNames; // �v���C���[���擾�����A�C�e�����ꗗ

	// �u�ǂ����v�ꗗ�̑I�𐧌�i�ǉ��j
	int subMenuSelection = 0;		 // �ꗗ���J�[�\��
	std::vector<int> subMenuItemIds; // �ꗗ�ɕ��ׂ� ItemID
	bool isItemListActive = false;	 // �ꗗ���쒆�t���O
									
	void RebuildItemList(); // �ǂ����ꗗ�̍č\�z�i���������ȂǂɌĂԁj



	std::mt19937 randomEngine;								  // ����������
	std::uniform_int_distribution<int> distribution{ 0, 99 }; // 0�`99 �̗���


	// ����}�b�v�J�ڎ��̃t�F�[�h�A�E�g����
	bool hasInitialFadeOut = false;
	bool initialFadeOutCompleted = false;

	// �t�F�[�h�C���E�t�F�[�h�A�E�g�p�ϐ�
	bool isFadingIn = false;
	bool isFadingOut = false;
	bool hasTransitioned = false;			 // �J�ڊ����t���O
	eSceneType nextScene = eSceneType::eMap; // ���̃V�[���^�C�v
	float fadeAlpha = 255.0f;				 // �t�F�[�h�̓����x
	const float fadeSpeed = 150.0f;			 // �t�F�[�h���x

	float encounterCooldown = 2.0f; // �N�[���^�C���i�b�j
	float encounterCooldownTimer = 0.0f;
	bool isAfterBattle = false;


	// ����X�|�[������i�ŏ��̃}�b�v���[�h���Ɏg�p�j
	bool isFirstSpawn = true;

	// �����_���G���J�E���g�̗L���E����
	bool isEncounterEnabled = true;

	// �t�F�[�h�C���J�n����
	void StartFadeIn();
	// �t�F�[�h�C���̍X�V����
	void UpdateFadeIn(float delta_second);
	// �t�F�[�h�C���`�揈��
	void DrawFadeIn();

	// �l���w��͈͓��Ɏ��߂�w���p�[�֐�
	template <typename T>
	T clamp(T value, T min, T max) {
		if (value < min)
			return min;
		if (value > max)
			return max;
		return value;
	}

public:
	Map();
	~Map();

	// ����������
	virtual void Initialize() override;

	int MapImage;				// �}�b�v�摜
	Vector2D generate_location; // �v���C���[�̏����ʒu
	class Player* player;		// �v���C���[�I�u�W�F�N�g

	// �X�V�����i�t�F�[�h�A�G���J�E���g�A�J�ڔ���Ȃǂ��܂ށj
	virtual eSceneType Update(float delta_second) override;

	// �`�揈���i�}�b�v�A�t�F�[�h���ʂȂǁj
	virtual void Draw() override;

	// �I�������i����������Ȃǁj
	virtual void Finalize() override;

	// ���݂̃V�[���^�C�v���擾
	virtual eSceneType GetNowSceneType() const override;

	// �}�b�v�f�[�^�̃��[�h�����iCSV�t�@�C������j
	std::vector<std::vector<char>> LoadStageMapCSV(std::string map_name);

	// �}�b�v�`�揈��
	void DrawStageMap();

	// ���̃}�b�v�����[�h
	void LoadNextMap();

	// �Փ˔���i�w����W�Ɉړ��\������j
	bool IsCollision(float x, float y);

	// ���݂̃}�b�v�f�[�^
	std::vector<std::vector<char>> mapdata;

private:
	int savedHp;
	int savedAttack;
	int savedDefense;
};
