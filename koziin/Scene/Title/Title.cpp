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
	TitleImage = rm->GetImages("Resource/Images/Title.png", 1, 1, 1, 32, 32)[0];
	select = rm->GetImages("Resource/Images/select.png", 1, 1, 1, 32, 32)[0];
	menu_cursor = 1;
}

eSceneType TitleScene::Update(float delta_second) 
{
	// “ü—Í‹@”\‚ÌŽæ“¾
	InputControl* input = Singleton<InputControl>::GetInstance();

	if (input->GetKeyDown(KEY_INPUT_DOWN))
	{
		menu_cursor++;

		if (menu_cursor > 4)
		{
			menu_cursor = 1;
		}
	}

	if (input->GetKeyDown(KEY_INPUT_UP)) {
		menu_cursor--;

		if (menu_cursor < 1) 
		{
			menu_cursor = 4;
		}
	}

	if (input->GetKeyDown(KEY_INPUT_SPACE))
	{
		if (menu_cursor == 1)
		{
			return eSceneType::eMap;
		}

		if (menu_cursor == 3)
		{
			return eSceneType::eOption;
		}

		if (menu_cursor == 4)
		{
			return eSceneType::eEnd;
		}

		
		
	}

	return GetNowSceneType();
}

void TitleScene::Draw() 
{
	DrawGraph(0, 0, TitleImage,TRUE);
	DrawRotaGraph(350, 430 + menu_cursor * 50, 0.1, 0, select, TRUE);
}

void TitleScene::Finalize() {
}

eSceneType TitleScene::GetNowSceneType() const 
{
	return eSceneType::eTitle;
}