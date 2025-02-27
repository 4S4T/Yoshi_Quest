#include "Battle.h"
#include "../../Utility/InputControl.h"
#include "DxLib.h"
#include "../SceneManager.h"
#include "../../Utility/ResourceManager.h"
#include"../../Object/Enemy/EnemyType/Slime.h"
#include"../../Object/GameObjectManager.h"


BattleScene::BattleScene() {}
BattleScene::~BattleScene() {}

void BattleScene::SetPlayerPosition(const Vector2D& position)
{
	playerPosition = position;
}

void BattleScene::Initialize()
{
	ResourceManager* rm = ResourceManager::GetInstance();
	generate_location = Vector2D(480.0f, 360.0f);
	GameManager* obj = Singleton<GameManager>::GetInstance();
	slime = obj->CreateGameObject<Slime>(generate_location);


}

eSceneType BattleScene::Update(float delta_second) 
{
	GameManager* gm = Singleton<GameManager>::GetInstance();
	gm->Update(delta_second);


	InputControl* input = Singleton<InputControl>::GetInstance();
	if (input->GetKey(KEY_INPUT_M)) 
	{
		return eSceneType::eMap;
	}

	//Draw();

	return GetNowSceneType();
}

void BattleScene::Draw()
{
	GameManager* gm = Singleton<GameManager>::GetInstance();
	gm->Draw();

	DrawBox(20, 690, 940, 710, GetColor(255, 255, 255), false);


	DrawString(0, 0, "バトル画面", GetColor(255, 255, 255)); // 白色で表示
}

void BattleScene::Finalize()
{
	GameManager* obj = Singleton<GameManager>::GetInstance();
	obj->Finalize();
}

eSceneType BattleScene::GetNowSceneType() const {
	return eSceneType::eBattle;
}