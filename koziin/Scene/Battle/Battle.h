#pragma once

#include "../SceneBase.h"
#include "../../Utility/Vector2D.h"
#include"../../Object/Enemy/EnemyType/Slime.h"
#include"../../Object/Enemy/EnemyType/peabird.h"
#include"../../Object/Enemy/EnemyType/Taurus.h"
#include"../Map/Map.h"


class BattleScene : public SceneBase {
private:
	Vector2D playerPosition; // �v���C���[�ʒu�ێ�
	Player* player;			  // Map����󂯎��v���C���[���
	bool battleEnded = false; // �퓬�I���t���O��ǉ�
	int cursor;
	int select;
	bool isPlayerTurn = true; // true�Ȃ�v���C���[�̃^�[���Afalse�Ȃ�G�̃^�[��
	std::string battleMessage; // �퓬���b�Z�[�W
	float messageTimer = 0.0f; // ���b�Z�[�W�\���p�^�C�}�[
							   // BattleScene�̃����o�ϐ��ɒǉ�
	float battleEndTimer = -1.0f; // -1: ���ݒ�, 0�ȏ�: �J�E���g�_�E����
	bool isSlimeDefeated = false;
	bool isTaurusDefeated = false;



public:
	BattleScene();
	~BattleScene();
	void SetPlayer(Player* p); // �v���C���[�f�[�^���Z�b�g����֐�

	void Initialize() override;
	eSceneType Update(float delta_second) override;
	void OnEnemyDefeated(const std::string& name, int exp);

	void Draw() override;
	void Finalize() override;
	eSceneType GetNowSceneType() const override;

	Vector2D generate_location;
	Vector2D player_location;
	Slime* slime;
	peabird* Peabird;
	Taurus* taurus;
	void SetPlayerPosition(const Vector2D& position); // �ʒu�ݒ胁�\�b�h
};