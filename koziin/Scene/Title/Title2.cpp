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
	cursor_SE = LoadSoundMem("Resource/Sounds/音/選択/選択1.mp3");

	ResourceManager* rm = ResourceManager::GetInstance();
	TitleImage = rm->GetImages("Resource/Images/Titel.png", 1, 1, 1, 32, 32)[0];
	Title_name = rm->GetImages("Resource/Images/3.png", 1, 1, 1, 32, 32)[0];
	select = rm->GetImages("Resource/Images/select.png", 1, 1, 1, 32, 32)[0];

	menu_cursor = 1;
}

eSceneType TitleScene2::Update(float delta_second) {
	InputControl* input = Singleton<InputControl>::GetInstance();


	//カーソル移動
	if (input->GetKeyDown(KEY_INPUT_DOWN)) {
		PlaySoundMem(cursor_SE, DX_PLAYTYPE_BACK);
		menu_cursor++;
		if (menu_cursor > 3)
			menu_cursor = 1;
	}

	if (input->GetKeyDown(KEY_INPUT_UP)) {
		PlaySoundMem(cursor_SE, DX_PLAYTYPE_BACK);
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
	// 背景
	DrawGraph(0, 0, TitleImage, TRUE);

	// タイトルロゴ
	DrawRotaGraph(500, 250, 0.5f, 0.0f, Title_name, TRUE);

	// セレクトカーソル
	DrawRotaGraph(350, 430 + menu_cursor * 50, 0.1f, 0.0f, select, TRUE);

	// フォント設定
	SetFontSize(30);

	// 点滅制御
	int time = GetNowCount();
	bool blink = (time / 500) % 2 == 0;

	// 色設定
	int blinkColor = blink
						 ? GetColor(255, 255, 100) // 選択中：黄色
						 : GetColor(255, 255, 255);

	int normalColor = GetColor(255, 255, 255); // 非選択：白
	int edgeColor = GetColor(0, 0, 0);		   // 黒フチ

	for (int i = 1; i <= 3; i++) {
		int x = 410;
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

		// ★ まず全項目に黒フチ
		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				if (dx == 0 && dy == 0)
					continue;
				DrawFormatString(x + dx, y + dy, edgeColor, text);
			}
		}

		// ★ 本体文字
		if (i == menu_cursor)
			DrawFormatString(x, y, blinkColor, text); // 選択中
		else
			DrawFormatString(x, y, normalColor, text); // 非選択
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
