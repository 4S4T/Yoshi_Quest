#include "../Title/Title.h"
#include "../../Utility/InputControl.h"
#include "DxLib.h"
#include "../SceneManager.h"
#include "../../Utility/ResourceManager.h"

TitleScene::TitleScene() {
}

TitleScene::~TitleScene() {
}

void TitleScene::Initialize()
{
	ResourceManager* rm = ResourceManager::GetInstance();
	TitleImage = rm->GetImages("Resource/Images/Title.jpg", 1, 1, 1, 32, 32)[0];
	Title_name = rm->GetImages("Resource/Images/2.png", 1, 1, 1, 32, 32)[0];
	select = rm->GetImages("Resource/Images/select.png", 1, 1, 1, 32, 32)[0];
	menu_cursor = 1;
}

eSceneType TitleScene::Update(float delta_second) 
{
	// 入力機能の取得
	InputControl* input = Singleton<InputControl>::GetInstance();

	/*if (input->GetKeyDown(KEY_INPUT_DOWN))
	{
		menu_cursor++;

		if (menu_cursor > 2)
		{
			menu_cursor = 1;
		}
	}

	if (input->GetKeyDown(KEY_INPUT_UP)) {
		menu_cursor--;

		if (menu_cursor < 1) 
		{
			menu_cursor = 2;
		}
	}*/

	if (input->GetKeyDown(KEY_INPUT_SPACE))
	{
		if (menu_cursor == 1)
		{
			return eSceneType::eTitle2;
		}

		/*if (menu_cursor == 2)
		{
			return eSceneType::eEnd;
		}*/

		
		
	}

	return GetNowSceneType();
}

void TitleScene::Draw() 
{
	DrawGraph(0, 0, TitleImage,TRUE);
	DrawRotaGraph(500, 250, 0.5f, 0.0, Title_name, TRUE);
	DrawRotaGraph(350, 430 + menu_cursor * 50, 0.1, 0, select, TRUE);
	SetFontSize(30);


	// 現在の経過時間（ミリ秒）
	int time = GetNowCount();

	// 0.5秒ごとにON/OFF切り替え
	bool blink = (time / 500) % 2 == 0;

	// 点滅色を切り替え（黄色⇔白）
	int color = blink ? GetColor(255, 255, 0) : GetColor(255, 255, 255);

	// 点滅描画
	DrawFormatString(380, 470, color, "STATE");
	
}

void TitleScene::Finalize() {
}

eSceneType TitleScene::GetNowSceneType() const 
{
	return eSceneType::eTitle;
}