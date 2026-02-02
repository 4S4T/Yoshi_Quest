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
	SoundManager::GetInstance().PlayBGM("Resource/Sounds/jo-jokyoku.mp3");
	cursorSound = LoadSoundMem("Resource/Sounds/Cursor.mp3");

	ResourceManager* rm = ResourceManager::GetInstance();
	TitleImage = rm->GetImages("Resource/Images/Titel.png", 1, 1, 1, 32, 32)[0];
	Title_name = rm->GetImages("Resource/Images/3.png", 1, 1, 1, 32, 32)[0];
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

	// 文字色（黄色 ⇄ 白）
	int mainColor = blink
						? GetColor(255, 255, 100)	   // 黄色
						: GetColor(255, 255, 255); // 白

	int edgeColor = GetColor(0, 0, 0); // 黒フチ

	int x = 410;
	int y = 470;

	// 黒フチ描画（8方向）
	for (int dx = -1; dx <= 1; dx++) {
		for (int dy = -1; dy <= 1; dy++) {
			if (dx == 0 && dy == 0)
				continue;
			DrawFormatString(x + dx, y + dy, edgeColor, "START");
		}
	}

	// 本体文字
	DrawFormatString(x, y, mainColor, "START");
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
