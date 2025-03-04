#include "Battle.h"
#include "../../Utility/InputControl.h"
#include "DxLib.h"
#include "../SceneManager.h"
#include "../../Utility/ResourceManager.h"
#include "../../Utility/PlayerData.h"
#include"../../Object/Enemy/EnemyType/Slime.h"
#include"../../Object/GameObjectManager.h"


BattleScene::BattleScene() 
{
	
}
BattleScene::~BattleScene() {}


void BattleScene::SetPlayer(Player* p) {

	this->player = p;
}


void BattleScene::Initialize()
{
	ResourceManager* rm = ResourceManager::GetInstance();
	generate_location = Vector2D(480.0f, 360.0f);
	GameManager* obj = Singleton<GameManager>::GetInstance();
	slime = obj->CreateGameObject<Slime>(generate_location);
	cursor = 1;
	select = rm->GetImages("Resource/Images/select.png", 1, 1, 1, 32, 32)[0];
	if (player == nullptr) {
		printf("Error: player is nullptr in BattleScene::Initialize()\n");
		return;
	}
	else {
		printf("Success: player is valid in BattleScene::Initialize()\n");
	}

	player->SetIsBattle(true);
}

eSceneType BattleScene::Update(float delta_second) {
	GameManager* gm = Singleton<GameManager>::GetInstance();
	gm->Update(delta_second);

	InputControl* input = Singleton<InputControl>::GetInstance();

	// �퓬���I�������ꍇ�A�}�b�v�ɖ߂�
	if (battleEnded)
	{
		return eSceneType::eMap;
	}

	if (isPlayerTurn)
	{
		// �퓬���̏���

		if (input->GetKeyDown(KEY_INPUT_DOWN)) {
		cursor = (cursor % 4) + 1; // 1�`4�͈̔͂Ń��[�v
	
		}

	
		if (input->GetKeyDown(KEY_INPUT_UP)) {
		cursor = (cursor == 1) ? 4 : cursor - 1;
	
		}

	
		if (input->GetKeyDown(KEY_INPUT_SPACE)) {
		
			if (cursor == 1) { // ��������
				PlayerData* pd = PlayerData::GetInstance();
				slime->SetHp(pd->GetAttack()); // �v���C���[�̍U���͂��擾���čU��
				isPlayerTurn = false;
			}
	

		
			if (cursor == 2) // �ڂ�����i�������j
		
			{
		
			}
		
			if (cursor == 3) // �ǂ����i�������j
		
			{
		
			}
		
			if (cursor == 4) // �ɂ���
		
			{
			
				battleEnded = true;
		
			}
	
		}
	
	}
	else
	{
		static float enemyWaitTime = 0.0f;
		enemyWaitTime += delta_second;

		if (enemyWaitTime >= 1.0f) {
			PlayerData* pd = PlayerData::GetInstance();
			pd->SetHp(pd->GetHp() - slime->GetAttack()); // HP������
			isPlayerTurn = true;
			enemyWaitTime = 0.0f;
			if (pd->GetHp() == 0)
			{
				return eSceneType::eEnd;
			}
		}
	}

	// �G��HP�� 0 �ȉ��ɂȂ�����퓬�I��
	if (slime->GetAHp() <= 0) {
		battleEnded = true;
	}


	Draw();

	return GetNowSceneType();
}


void BattleScene::Draw()
{
	GameManager* gm = Singleton<GameManager>::GetInstance();
	gm->Draw();

	DrawBox(20, 550, 150, 700, GetColor(255, 255, 255), false);
	DrawBox(200, 550, 900, 700, GetColor(255, 255, 255), false);

	DrawString(0, 0, "�o�g�����", GetColor(255, 255, 255)); // ���F�ŕ\��
	DrawString(70, 570, "��������", GetColor(255, 255, 255)); // ���F�ŕ\��
	DrawString(70, 600, "�ڂ�����", GetColor(255, 255, 255));	 // ���F�ŕ\��
	DrawString(70, 630, "�ǂ���", GetColor(255, 255, 255)); // ���F�ŕ\��
	DrawString(70, 660 , "�ɂ���", GetColor(255, 255, 255));	  // ���F�ŕ\��
	DrawRotaGraph(40, 540 + cursor * 30, 0.05, 0, select, TRUE);

	PlayerData* pd = PlayerData::GetInstance();
	int PlayerHp = pd->GetHp();

	DrawFormatString(0, 300, GetColor(255, 255, 255), "PlayerHp�@: %d", PlayerHp);
	
}

void BattleScene::Finalize()
{
	GameManager* obj = Singleton<GameManager>::GetInstance();
	obj->Finalize();
}

eSceneType BattleScene::GetNowSceneType() const {
	return eSceneType::eBattle;
}

void BattleScene::SetPlayerPosition(const Vector2D& position) 
{
	playerPosition = position;
}
