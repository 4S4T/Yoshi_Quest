#include "../Title/Title.h"
#include "../../Utility/InputControl.h"
#include "DxLib.h"
#include "../SceneManager.h"
#include "../../Utility/ResourceManager.h"
#include "../../Utility/SoundManager.h" // ← BGM制御追加

TitleScene::TitleScene() {}
TitleScene::~TitleScene() {}

void TitleScene::Initialize()
{
	// ★ タイトルBGM再生（シーン移動してもこのシーンの間だけ再生）
	SoundManager::GetInstance().PlayBGM("Resource/Sounds/Title.mp3");
	cursorSound = LoadSoundMem("Resource/Sounds/Cursor.mp3");

	ResourceManager* rm = ResourceManager::GetInstance();
	TitleImage = rm->GetImages("Resource/Images/Title.jpg", 1, 1, 1, 32, 32)[0];
	Title_name = rm->GetImages("Resource/Images/2.png", 1, 1, 1, 32, 32)[0];
	select = rm->GetImages("Resource/Images/select.png", 1, 1, 1, 32, 32)[0];
	menu_cursor = 1;
}

eSceneType TitleScene::Update(float delta_second) {
	InputControl* input = Singleton<InputControl>::GetInstance();

	if (input->GetKeyDown(KEY_INPUT_SPACE)) {

		if (cursorSound != -1)
			PlaySoundMem(cursorSound, DX_PLAYTYPE_NORMAL);

		if (menu_cursor == 1)
		{
			return eSceneType::eTitle2;
		}
	}

	return GetNowSceneType();
}

void TitleScene::Draw() {
	DrawGraph(0, 0, TitleImage, TRUE);
	DrawRotaGraph(500, 250, 0.5f, 0.0, Title_name, TRUE);
	DrawRotaGraph(350, 430 + menu_cursor * 50, 0.1f, 0.0f, select, TRUE);

	SetFontSize(30);

	int time = GetNowCount();
	bool blink = (time / 500) % 2 == 0;
	int color = blink ? GetColor(255, 255, 0) : GetColor(255, 255, 255);

	DrawFormatString(380, 470, color, "STATE");
}

void TitleScene::Finalize() {
	// サウンドハンドルを削除
	if (cursorSound != -1) {
		DeleteSoundMem(cursorSound);
		cursorSound = -1;
	}
}

eSceneType TitleScene::GetNowSceneType() const {
	return eSceneType::eTitle;
}
