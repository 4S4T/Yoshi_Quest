#include "Battle.h"
#include "../../Utility/InputControl.h"
#include "DxLib.h"
#include "../SceneManager.h"
#include "../../Utility/ResourceManager.h"
#include "../../Utility/PlayerData.h"
#include"../../Object/Enemy/EnemyType/Slime.h"
#include"../../Object/GameObjectManager.h"
#include<string>



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

	//// �퓬���I�������ꍇ�A�}�b�v�ɖ߂�
	//if (battleEnded)
	//{
	//	return eSceneType::eMap;
	//}

	// ���b�Z�[�W�\�����͑I�����𑀍�ł��Ȃ��悤�ɂ���
	if (messageTimer > 0.0f) 
	{
		messageTimer -= delta_second; // ���b�Z�[�W�\�����Ԃ𒲐�
		return GetNowSceneType();
	}

	if (isPlayerTurn)
	{
		// �퓬���̏���

		if (input->GetKeyDown(KEY_INPUT_DOWN)) {
			cursor++;
			if (cursor > 4)
				cursor = 1;
		}

		if (input->GetKeyDown(KEY_INPUT_UP)) {
			cursor--;
			if (cursor < 1)
				cursor = 4;
		}


	
		if (input->GetKeyDown(KEY_INPUT_SPACE)) {
		
			if (cursor == 1) { // ��������
				PlayerData* pd = PlayerData::GetInstance();
				int damage = pd->GetAttack();
				slime->SetHp(damage); // �_���[�W��^����

				// ���b�Z�[�W���Z�b�g
				battleMessage = "������[�̍U���I �X���C���� " + std::to_string(damage) + " �̃_���[�W�I";
				messageTimer = 2.0f; // 2�b�ԕ\������
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
				return eSceneType::eMap;
		
			}
	
		}
	
	}
	else
	{
		// �X���C�����|���ꂽ��A�G�^�[���̏������s��Ȃ�
		if (slime->GetHp() > 0) { // �X���C�����|��Ă��Ȃ��ꍇ�̂ݓG�^�[��
			static float enemyWaitTime = 0.0f;
			enemyWaitTime += delta_second;

			if (enemyWaitTime >= 1.0f) {
				PlayerData* pd = PlayerData::GetInstance();
				int damage = slime->GetAttack(); // �X���C���̍U���͂��擾
				pd->SetHp(pd->GetHp() - damage); // HP������

				// �G�^�[���̃��b�Z�[�W
				battleMessage = "�X���C���̍U���I ������[�� " + std::to_string(damage) + " �̃_���[�W�I";
				messageTimer = 5.0f; // 2�b�ԕ\��

				isPlayerTurn = true;
				enemyWaitTime = 0.0f;

				if (pd->GetHp() <= 0) {
					return eSceneType::eEnd;
				}
			}
		}
	} 

	// �X���C����|�����ꍇ
	if (slime->GetHp() <= 0 && !isSlimeDefeated) {
		// �X���C����|�����ꍇ�Ɍo���l��t�^
		PlayerData* pd = PlayerData::GetInstance();
		pd->AddExperience(100); // �Ⴆ�� 100 XP ��ǉ�

		// �J�ڂ�x�点�邽�߂̃^�C�}�[���Z�b�g
		battleEndTimer = 1.0f; // 2�b�ԑ҂�
		battleMessage = "�X���C����|�����I �o���l�� 100 �l�������I";

		isSlimeDefeated = true;
	}

	// �퓬�I�����̒x������
	if (battleEndTimer > 0.0f) {
		battleEndTimer -= delta_second;					// �^�C�}�[������
		printf("battleEndTimer: %f\n", battleEndTimer); // �f�o�b�O�\��

		// battleEndTimer��0�ȉ�����messageTimer���I��������J��
		if ( battleEndTimer <= 0.0f && messageTimer <= 0.0f) {
			return eSceneType::eMap; // ��莞�Ԍ�ɑJ��
		}
	}




	Draw();

	return GetNowSceneType();
}


void BattleScene::Draw()
{
	GameManager* gm = Singleton<GameManager>::GetInstance();
	gm->Draw();
	
	// �傫���t�H���g���쐬�i�T�C�Y30�j
	int LargeFont = CreateFontToHandle("�l�r �S�V�b�N", 30, 6);

	// �{�b�N�X�̕`��
	DrawBox(20, 530, 190, 700, GetColor(255, 255, 255), false);
	DrawBox(200, 530, 900, 700, GetColor(255, 255, 255), false);
	DrawBox(0, 20, 150, 170, GetColor(255, 255, 255), false);
	DrawBox(0, 20, 150, 170, GetColor(255, 255, 255), false);
	DrawBox(37.5, 10, 112.5, 40, GetColor(255, 255, 255), true);

	// �������t�H���g�̂܂܂ŕ\��
	DrawString(37.5, 20, "������[", GetColor(0, 0, 0));

	// **��������������傫��**
	DrawStringToHandle(50, 540, "��������", GetColor(255, 255, 255), LargeFont);
	DrawStringToHandle(50, 580, "�ڂ�����", GetColor(255, 255, 255), LargeFont);
	DrawStringToHandle(50, 620, "�ǂ���", GetColor(255, 255, 255), LargeFont);
	DrawStringToHandle(50, 660, "�ɂ���", GetColor(255, 255, 255), LargeFont);

	PlayerData* pd = PlayerData::GetInstance();
	int PlayerHp = pd->GetHp();
	int PlayerLevel = pd->GetLevel();
	// �傫���t�H���g�� HP ��\��
	
	DrawFormatStringToHandle(5, 50, GetColor(255, 255, 255), LargeFont, "Lv�@:  %d", PlayerLevel);
	DrawFormatStringToHandle(5, 90, GetColor(255, 255, 255), LargeFont, "Hp�@:  %d", PlayerHp);


	// �J�[�\���̕`��
	DrawRotaGraph(40, 555 + (cursor - 1) * 40, 0.05, 0, select, TRUE);

	// �g�p�����t�H���g������i�������Ǘ��j
	DeleteFontToHandle(LargeFont);


	


	
	// �o�g�����b�Z�[�W��`��
	if (messageTimer > 0.0f) 
	{
		DrawString(250, 600, battleMessage.c_str(), GetColor(255, 255, 255)); // ���F�Ń��b�Z�[�W��\��
		messageTimer -= 0.001f;												  // ���b�Z�[�W�\�����Ԃ����炷
	}

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
