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


PlayerCommandState commandState = PlayerCommandState::MainCommand;

void BattleScene::SetPlayer(Player* p) {
	this->player = p;
}

void BattleScene::Initialize() {
	ResourceManager* rm = ResourceManager::GetInstance();
	GameManager* obj = Singleton<GameManager>::GetInstance();
	generate_location = Vector2D(480.0f, 360.0f);

	// 敵生成
	taurus = obj->CreateGameObject<Taurus>(Vector2D(580.0f, 300.0f));
	Peabird = obj->CreateGameObject<peabird>(Vector2D(380.0f, 300.0f));

	// カーソル・選択画像読み込み
	cursor = 1;
	select = rm->GetImages("Resource/Images/select.png", 1, 1, 1, 32, 32)[0];

	if (player == nullptr) {
		printf("Error: player is nullptr in BattleScene::Initialize()\n");
		return;
	}
	player->SetIsBattle(true);

	// 初期化
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

	static int earnedExp = 0;
	static std::vector<std::pair<std::string, int>> defeatedEnemies;
	static size_t messageIndex = 0;

	if (messageTimer > 0.0f) {
		messageTimer -= delta_second;
		return GetNowSceneType();
	}

	if (isPlayerTurn) {
		switch (commandState) {
		case PlayerCommandState::MainCommand:
			// メインコマンド
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
			if (input->GetKeyDown(KEY_INPUT_SPACE)) {
				if (cursor == 1) {
					commandState = PlayerCommandState::AttackTarget;
					cursor = 1;
				}
				else if (cursor == 2) {
					return eSceneType::eMap;
				}
			}
			break;

		case PlayerCommandState::AttackTarget:
			// 攻撃対象選択
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

			// 攻撃実行
			if (input->GetKeyDown(KEY_INPUT_SPACE)) {
				if (cursor == 1 && Peabird->GetHp() > 0) {
					int rawDamage = PlayerData::GetInstance()->GetAttack() / 2;
					int actualDamage = rawDamage - Peabird->GetDefense() / 4;
					if (actualDamage < 0)
						actualDamage = 0;
					Peabird->SetHp(actualDamage);
					Peabird->SetBlink(1.0f);
					battleMessage = "よっしーの攻撃！トリッピーに " + std::to_string(actualDamage) + " のダメージ！";
					messageTimer = 2.0f;
					isPlayerTurn = false;
					commandState = PlayerCommandState::MainCommand;
				}
				else if (cursor == 2 && taurus->GetHp() > 0) {
					int rawDamage = PlayerData::GetInstance()->GetAttack() / 2;
					int actualDamage = rawDamage - taurus->GetDefense() / 4;
					if (actualDamage < 0)
						actualDamage = 0;
					taurus->SetHp(actualDamage);
					taurus->SetBlink(1.0f);
					battleMessage = "よっしーの攻撃！タウロスに " + std::to_string(actualDamage) + " のダメージ！";
					messageTimer = 2.0f;
					isPlayerTurn = false;
					commandState = PlayerCommandState::MainCommand;
				}
			}
			break;
		}
	}
	else {
		// 敵ターン
		static float enemyWaitTime = 0.0f;
		enemyWaitTime += delta_second;

		if (enemyWaitTime >= 1.0f) {
			PlayerData* pd = PlayerData::GetInstance();
			if (Peabird->GetHp() > 0) {
				int rawDamage = Peabird->GetAttack() / 2;
				pd->SetHp(pd->GetHp() - rawDamage);
				battleMessage = "トリッピーの攻撃！よっしーに " + std::to_string(rawDamage) + " のダメージ！";
			}
			else if (taurus->GetHp() > 0) {
				int rawDamage = taurus->GetAttack() / 2;
				pd->SetHp(pd->GetHp() - rawDamage);
				battleMessage = "タウロスの攻撃！よっしーに " + std::to_string(rawDamage) + " のダメージ！";
			}
			messageTimer = 2.0f;
			isPlayerTurn = true;
			enemyWaitTime = 0.0f;

			if (pd->GetHp() <= 0)
				return eSceneType::eEnd;
		}
	}

	// 経験値処理
	PlayerData* pd = PlayerData::GetInstance();

	if (Peabird->GetHp() <= 0 && !isSlimeDefeated) {
		isSlimeDefeated = true;
		Peabird->SetVisible(false);
		defeatedEnemies.push_back({ "トリッピー", 100 });
	}
	if (taurus->GetHp() <= 0 && !isTaurusDefeated) {
		isTaurusDefeated = true;
		taurus->SetVisible(false);
		defeatedEnemies.push_back({ "タウロス", 150 });
	}

	if (isSlimeDefeated && isTaurusDefeated && battleEndTimer < 0.0f) {
		battleEndTimer = 2.0f;
		messageIndex = 0;
	}

	if (battleEndTimer > 0.0f) {
		battleEndTimer -= delta_second;

		if (!defeatedEnemies.empty()) {
			if (messageIndex < defeatedEnemies.size() && messageTimer <= 0.0f) {
				auto& e = defeatedEnemies[messageIndex];
				battleMessage = e.first + "を倒した！経験値" + std::to_string(e.second) + "獲得！";
				earnedExp += e.second;
				messageTimer = 2.0f;
				messageIndex++;
			}
			else if (messageIndex >= defeatedEnemies.size() && messageTimer <= 0.0f) {
				pd->AddExperience(earnedExp);
				earnedExp = 0;
				defeatedEnemies.clear();
				messageIndex = 0;
				return eSceneType::eMap;
			}
		}
	}

	Draw();
	return GetNowSceneType();
}

void BattleScene::Draw() {
	GameManager* gm = Singleton<GameManager>::GetInstance();
	gm->Draw();

	int LargeFont = CreateFontToHandle("ＭＳ ゴシック", 18, 6);

	// UI
	DrawBox(20, 530, 190, 700, GetColor(255, 255, 255), false);
	DrawBox(200, 530, 900, 700, GetColor(255, 255, 255), false);
	DrawBox(0, 20, 150, 170, GetColor(255, 255, 255), false);
	DrawBox(37, 10, 112, 40, GetColor(255, 255, 255), true);
	DrawString(38, 20, "よっしー", GetColor(0, 0, 0));

	if (commandState == PlayerCommandState::MainCommand) {
		DrawStringToHandle(50, 540, "たたかう", GetColor(255, 255, 255), LargeFont);
		DrawStringToHandle(50, 580, "にげる", GetColor(255, 255, 255), LargeFont);
		DrawRotaGraph(40, 555 + (cursor - 1) * 40, 0.05, 0, select, TRUE);
	}
	else if (commandState == PlayerCommandState::AttackTarget) {
		DrawStringToHandle(50, 540, "トリッピーを攻撃", GetColor(255, 255, 255), LargeFont);
		DrawStringToHandle(50, 580, "タウロスを攻撃", GetColor(255, 255, 255), LargeFont);
		DrawRotaGraph(40, 555 + (cursor - 1) * 40, 0.05, 0, select, TRUE);
	}

	PlayerData* pd = PlayerData::GetInstance();
	DrawFormatStringToHandle(5, 50, GetColor(255, 255, 255), LargeFont, "Lv　:  %d", pd->GetLevel());
	DrawFormatStringToHandle(5, 90, GetColor(255, 255, 255), LargeFont, "Hp　:  %d", pd->GetHp());

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
