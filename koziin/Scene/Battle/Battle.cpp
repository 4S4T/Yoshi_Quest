#include "Battle.h"
#include "../../Utility/InputControl.h"
#include "DxLib.h"
#include "../SceneManager.h"
#include "../../Utility/ResourceManager.h"
#include "../../Utility/PlayerData.h"
#include "../../Object/Enemy/EnemyType/Slime.h"
#include "../../Object/Enemy/EnemyType/Taurus.h"
#include "../../Object/GameObjectManager.h"
#include <string>

BattleScene::BattleScene() {}

BattleScene::~BattleScene() {}

void BattleScene::SetPlayer(Player* p) {
	this->player = p;
}

void BattleScene::Initialize() {
	ResourceManager* rm = ResourceManager::GetInstance();
	GameManager* obj = Singleton<GameManager>::GetInstance();
	generate_location = Vector2D(480.0f, 360.0f);

	// �X���C���ƃ^�E���X�𐶐����Ĉʒu�����炷
	slime = obj->CreateGameObject<Slime>(Vector2D(380.0f, 300.0f));
	taurus = obj->CreateGameObject<Taurus>(Vector2D(580.0f, 300.0f));

	// �J�[�\���E�I���摜�ǂݍ���
	cursor = 1;
	select = rm->GetImages("Resource/Images/select.png", 1, 1, 1, 32, 32)[0];

	if (player == nullptr) {
		printf("Error: player is nullptr in BattleScene::Initialize()\n");
		return;
	}
	player->SetIsBattle(true);

	// ������
	isPlayerTurn = true;
	battleMessage = "";
	messageTimer = 0.0f;
	battleEndTimer = -1.0f;
	isSlimeDefeated = false;
	isTaurusDefeated = false;
}

eSceneType BattleScene::Update(float delta_second) {
	GameManager* gm = Singleton<GameManager>::GetInstance();
	gm->Update(delta_second);

	InputControl* input = Singleton<InputControl>::GetInstance();

	if (messageTimer > 0.0f) {
		messageTimer -= delta_second;
		return GetNowSceneType();
	}

	if (isPlayerTurn) {
		if (input->GetKeyDown(KEY_INPUT_DOWN)) {
			cursor++;
			if (cursor > 2)
				cursor = 1;
		}
		if (input->GetKeyDown(KEY_INPUT_UP)) {
			cursor--;
			if (cursor < 1)
				cursor = 2;
		}

		// �X�y�[�X�L�[�ōU�������s
		if (input->GetKeyDown(KEY_INPUT_SPACE)) {
			if (cursor == 1 && slime->GetHp() > 0) {
				int rawDamage = PlayerData::GetInstance()->GetAttack();
				int actualDamage = rawDamage - slime->GetDefense();
				if (actualDamage < 0)
					actualDamage = 0;

				slime->SetHp(actualDamage);
				slime->SetBlink(1.0f); // �_�ŃA�j���[�V����
				battleMessage = "������[�̍U���I�X���C���� " + std::to_string(actualDamage) + " �̃_���[�W�I";
				messageTimer = 2.0f;
				isPlayerTurn = false;
			}

			if (cursor == 2 && taurus->GetHp() > 0) {
				int rawDamage = PlayerData::GetInstance()->GetAttack();
				int actualDamage = rawDamage - taurus->GetDefense();
				if (actualDamage < 0)
					actualDamage = 0;

				taurus->SetHp(actualDamage);
				taurus->SetBlink(1.0f); // �_�ŃA�j���[�V����
				battleMessage = "������[�̍U���I�^�E���X�� " + std::to_string(actualDamage) + " �̃_���[�W�I";
				messageTimer = 2.0f;
				isPlayerTurn = false;
			}
		}
	}


	else {
		// �G�^�[���F���ꂼ��̓G�������Ă���΍U��
		static float enemyWaitTime = 0.0f;
		enemyWaitTime += delta_second;

		if (enemyWaitTime >= 1.0f) {
			PlayerData* pd = PlayerData::GetInstance();
			if (slime->GetHp() > 0) {
				int damage = slime->GetAttack();
				pd->SetHp(pd->GetHp() - damage);
				battleMessage = "�X���C���̍U���I������[�� " + std::to_string(damage) + " �̃_���[�W�I";
			}
			else if (taurus->GetHp() > 0) {
				int damage = taurus->GetAttack();
				pd->SetHp(pd->GetHp() - damage);
				battleMessage = "�^�E���X�̍U���I������[�� " + std::to_string(damage) + " �̃_���[�W�I";
			}
			messageTimer = 2.0f;
			isPlayerTurn = true;
			enemyWaitTime = 0.0f;

			if (pd->GetHp() <= 0)
				return eSceneType::eEnd;
		}
	}

	// �o���l�Ə��������i�ʂɁj
	PlayerData* pd = PlayerData::GetInstance();
	if (slime->GetHp() <= 0 && !isSlimeDefeated) {
		pd->AddExperience(100);
		battleMessage = "�X���C����|�����I�o���l��100�l���I";
		messageTimer = 2.0f;
		isSlimeDefeated = true;
		slime->SetVisible(false);
	}
	if (taurus->GetHp() <= 0 && !isTaurusDefeated) {
		pd->AddExperience(150);
		battleMessage = "�^�E���X��|�����I�o���l��150�l���I";
		messageTimer = 2.0f;
		isTaurusDefeated = true;
		taurus->SetVisible(false);
	}

	// �����|������I��
	if (isSlimeDefeated && isTaurusDefeated && battleEndTimer < 0.0f) {
		battleEndTimer = 2.0f;
	}
	if (battleEndTimer > 0.0f) {
		battleEndTimer -= delta_second;
		if (battleEndTimer <= 0.0f && messageTimer <= 0.0f)
			return eSceneType::eMap;
	}

	Draw();
	return GetNowSceneType();
}

void BattleScene::Draw() {
	GameManager* gm = Singleton<GameManager>::GetInstance();
	gm->Draw();

	int LargeFont = CreateFontToHandle("�l�r �S�V�b�N", 18, 6);

	// UI�`��
	DrawBox(20, 530, 190, 700, GetColor(255, 255, 255), false);
	DrawBox(200, 530, 900, 700, GetColor(255, 255, 255), false);
	DrawBox(0, 20, 150, 170, GetColor(255, 255, 255), false);
	DrawBox(37.5, 10, 112.5, 40, GetColor(255, 255, 255), true);
	DrawString(37.5, 20, "������[", GetColor(0, 0, 0));

	DrawStringToHandle(50, 540, "�X���C�����U��", GetColor(255, 255, 255), LargeFont);
	DrawStringToHandle(50, 580, "�^�E���X���U��", GetColor(255, 255, 255), LargeFont);

	PlayerData* pd = PlayerData::GetInstance();
	DrawFormatStringToHandle(5, 50, GetColor(255, 255, 255), LargeFont, "Lv�@:  %d", pd->GetLevel());
	DrawFormatStringToHandle(5, 90, GetColor(255, 255, 255), LargeFont, "Hp�@:  %d", pd->GetHp());

	// �J�[�\��
	DrawRotaGraph(40, 555 + (cursor - 1) * 40, 0.05, 0, select, TRUE);

	// ���b�Z�[�W
	if (messageTimer > 0.0f) {
		DrawString(250, 600, battleMessage.c_str(), GetColor(255, 255, 255));
	}

	DeleteFontToHandle(LargeFont);
}

void BattleScene::Finalize() {
	GameManager* obj = Singleton<GameManager>::GetInstance();
	obj->Finalize();
}

eSceneType BattleScene::GetNowSceneType() const {
	return eSceneType::eBattle;
}

void BattleScene::SetPlayerPosition(const Vector2D& position) {
	playerPosition = position;
}
