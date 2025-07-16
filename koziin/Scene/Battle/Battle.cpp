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

	//// 戦闘が終了した場合、マップに戻る
	//if (battleEnded)
	//{
	//	return eSceneType::eMap;
	//}

	// メッセージ表示中は選択肢を操作できないようにする
	if (messageTimer > 0.0f) 
	{
		messageTimer -= delta_second; // メッセージ表示時間を調整
		return GetNowSceneType();
	}

	if (isPlayerTurn)
	{
		// 戦闘中の処理

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
		
			if (cursor == 1) { // たたかう
				PlayerData* pd = PlayerData::GetInstance();
				int damage = pd->GetAttack();
				slime->SetHp(damage); // ダメージを与える

				// メッセージをセット
				battleMessage = "よっしーの攻撃！ スライムに " + std::to_string(damage) + " のダメージ！";
				messageTimer = 2.0f; // 2秒間表示する
				isPlayerTurn = false;
			}


		
			if (cursor == 2) // ぼうぎょ（未実装）
		
			{
		
			}
		
			if (cursor == 3) // どうぐ（未実装）
		
			{
		
			}
		
			if (cursor == 4) // にげる
		
			{
			
				battleEnded = true;
				return eSceneType::eMap;
		
			}
	
		}
	
	}
	else
	{
		// スライムが倒されたら、敵ターンの処理を行わない
		if (slime->GetHp() > 0) { // スライムが倒れていない場合のみ敵ターン
			static float enemyWaitTime = 0.0f;
			enemyWaitTime += delta_second;

			if (enemyWaitTime >= 1.0f) {
				PlayerData* pd = PlayerData::GetInstance();
				int damage = slime->GetAttack(); // スライムの攻撃力を取得
				pd->SetHp(pd->GetHp() - damage); // HPを減少

				// 敵ターンのメッセージ
				battleMessage = "スライムの攻撃！ よっしーに " + std::to_string(damage) + " のダメージ！";
				messageTimer = 5.0f; // 2秒間表示

				isPlayerTurn = true;
				enemyWaitTime = 0.0f;

				if (pd->GetHp() <= 0) {
					return eSceneType::eEnd;
				}
			}
		}
	} 

	// スライムを倒した場合
	if (slime->GetHp() <= 0 && !isSlimeDefeated) {
		// スライムを倒した場合に経験値を付与
		PlayerData* pd = PlayerData::GetInstance();
		pd->AddExperience(100); // 例えば 100 XP を追加

		// 遷移を遅らせるためのタイマーをセット
		battleEndTimer = 1.0f; // 2秒間待つ
		battleMessage = "スライムを倒した！ 経験値を 100 獲得した！";

		isSlimeDefeated = true;
	}

	// 戦闘終了時の遅延処理
	if (battleEndTimer > 0.0f) {
		battleEndTimer -= delta_second;					// タイマーを減少
		printf("battleEndTimer: %f\n", battleEndTimer); // デバッグ表示

		// battleEndTimerが0以下かつmessageTimerが終了したら遷移
		if ( battleEndTimer <= 0.0f && messageTimer <= 0.0f) {
			return eSceneType::eMap; // 一定時間後に遷移
		}
	}




	Draw();

	return GetNowSceneType();
}


void BattleScene::Draw()
{
	GameManager* gm = Singleton<GameManager>::GetInstance();
	gm->Draw();
	
	// 大きいフォントを作成（サイズ30）
	int LargeFont = CreateFontToHandle("ＭＳ ゴシック", 30, 6);

	// ボックスの描画
	DrawBox(20, 530, 190, 700, GetColor(255, 255, 255), false);
	DrawBox(200, 530, 900, 700, GetColor(255, 255, 255), false);
	DrawBox(0, 20, 150, 170, GetColor(255, 255, 255), false);
	DrawBox(0, 20, 150, 170, GetColor(255, 255, 255), false);
	DrawBox(37.5, 10, 112.5, 40, GetColor(255, 255, 255), true);

	// 小さいフォントのままで表示
	DrawString(37.5, 20, "よっしー", GetColor(0, 0, 0));

	// **ここだけ文字を大きく**
	DrawStringToHandle(50, 540, "たたかう", GetColor(255, 255, 255), LargeFont);
	DrawStringToHandle(50, 580, "ぼうぎょ", GetColor(255, 255, 255), LargeFont);
	DrawStringToHandle(50, 620, "どうぐ", GetColor(255, 255, 255), LargeFont);
	DrawStringToHandle(50, 660, "にげる", GetColor(255, 255, 255), LargeFont);

	PlayerData* pd = PlayerData::GetInstance();
	int PlayerHp = pd->GetHp();
	int PlayerLevel = pd->GetLevel();
	// 大きいフォントで HP を表示
	
	DrawFormatStringToHandle(5, 50, GetColor(255, 255, 255), LargeFont, "Lv　:  %d", PlayerLevel);
	DrawFormatStringToHandle(5, 90, GetColor(255, 255, 255), LargeFont, "Hp　:  %d", PlayerHp);


	// カーソルの描画
	DrawRotaGraph(40, 555 + (cursor - 1) * 40, 0.05, 0, select, TRUE);

	// 使用したフォントを解放（メモリ管理）
	DeleteFontToHandle(LargeFont);


	


	
	// バトルメッセージを描画
	if (messageTimer > 0.0f) 
	{
		DrawString(250, 600, battleMessage.c_str(), GetColor(255, 255, 255)); // 白色でメッセージを表示
		messageTimer -= 0.001f;												  // メッセージ表示時間を減らす
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
