#pragma once

#include "../SceneBase.h"
#include "../../Utility/Vector2D.h"
#include"../../Object/Enemy/EnemyType/Slime.h"
#include"../Map/Map.h"


class BattleScene : public SceneBase {
private:
	Vector2D playerPosition; // �v���C���[�ʒu�ێ�
	Player* player;			  // Map����󂯎��v���C���[���
	bool battleEnded = false; // �퓬�I���t���O��ǉ�
	int cursor;
	int select;
	bool isPlayerTurn = true; // true�Ȃ�v���C���[�̃^�[���Afalse�Ȃ�G�̃^�[��


public:
	BattleScene();
	~BattleScene();
	void SetPlayer(Player* p); // �v���C���[�f�[�^���Z�b�g����֐�

	void Initialize() override;
	eSceneType Update(float delta_second) override;
	void Draw() override;
	void Finalize() override;
	eSceneType GetNowSceneType() const override;

	Vector2D generate_location;
	Vector2D player_location;
	Slime* slime;
	void SetPlayerPosition(const Vector2D& position); // �ʒu�ݒ胁�\�b�h
};