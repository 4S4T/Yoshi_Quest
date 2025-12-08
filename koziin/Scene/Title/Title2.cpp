#include "../Title/Title2.h"
#include "../../Utility/InputControl.h"
#include "DxLib.h"
#include "../SceneManager.h"
#include "../../Utility/ResourceManager.h"
#include "../../Utility/SoundManager.h" // ← BGM制御追加

TitleScene2::TitleScene2() {}
TitleScene2::~TitleScene2() {}

void TitleScene2::Initialize() {
	// ★ Title2でも同じタイトルBGMを使用
	SoundManager::GetInstance().PlayBGM("Resource/Sounds/Title.mp3");
	cursorSound2 = LoadSoundMem("Resource/Sounds/Cursor.mp3");

	ResourceManager* rm = ResourceManager::GetInstance();
	TitleImage = rm->GetImages("Resource/Images/Title.jpg", 1, 1, 1, 32, 32)[0];
	Title_name = rm->GetImages("Resource/Images/2.png", 1, 1, 1, 32, 32)[0];
	select = rm->GetImages("Resource/Images/select.png", 1, 1, 1, 32, 32)[0];

	menu_cursor = 1;
}

eSceneType TitleScene2::Update(float delta_second) {
	InputControl* input = Singleton<InputControl>::GetInstance();


	//カーソル移動
	if (input->GetKeyDown(KEY_INPUT_DOWN)) {
		menu_cursor++;
		if (menu_cursor > 3)
			menu_cursor = 1;
	}

	if (input->GetKeyDown(KEY_INPUT_UP)) {
		menu_cursor--;
		if (menu_cursor < 1)
			menu_cursor = 3;
	}

	 //シーン移動
	if (input->GetKeyDown(KEY_INPUT_SPACE)) {
		
		if (cursorSound2 != -1)
			PlaySoundMem(cursorSound2, DX_PLAYTYPE_NORMAL);

		//マップ
		if (menu_cursor == 1) {
			SoundManager::GetInstance().StopBGM(); //BGMを止める
			return eSceneType::eMap;
		}

		//オプション
		if (menu_cursor == 2) {
			return eSceneType::eOption;
		}

		//エンド
		if (menu_cursor == 3) {
			SoundManager::GetInstance().StopBGM();
			return eSceneType::eEnd;
		}
	}
	return GetNowSceneType();
}

void TitleScene2::Draw() {
	DrawGraph(0, 0, TitleImage, TRUE);
	DrawRotaGraph(500, 250, 0.5f, 0.0, Title_name, TRUE);
	DrawRotaGraph(350, 430 + menu_cursor * 50, 0.1, 0, select, TRUE);

	int time = GetNowCount();
	bool blink = (time / 500) % 2 == 0;
	int colorBlink = blink ? GetColor(255, 255, 0) : GetColor(255, 255, 255);

	SetFontSize(30);

	for (int i = 1; i <= 3; i++) {
		int y = 470 + (i - 1) * 50;
		const char* text = "";

		switch (i) {
		case 1:
			text = "STATE";
			break;
		case 2:
			text = "OPTION";
			break;
		case 3:
			text = "EXIT";
			break;
		}

		if (i == menu_cursor)
			DrawFormatString(380, y, colorBlink, text);
		else
			DrawFormatString(380, y, GetColor(255, 255, 255), text);
	}
}

void TitleScene2::Finalize() {
	SetFontSize(19);

	if (cursorSound2 != -1) {
		DeleteSoundMem(cursorSound2);
		cursorSound2 = -1;
	}


	//// ★ Title2 を抜ける時には必ず停止
	//SoundManager::GetInstance().StopBGM();
}

eSceneType TitleScene2::GetNowSceneType() const {
	return eSceneType::eTitle2;
}
